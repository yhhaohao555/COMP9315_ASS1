/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;

typedef struct PersonName
{
	int length;
	char name[1]
}			PersonName;


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(pname_in);

Datum
pname_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);
	
	PersonName *result;

	char ebuff[256];
    int ret;
    int cflags;
    regex_t reg;

    cflags = REG_EXTENDED | REG_NOSUB;


    char* test_str = "S Lili Beaty, John Lee Sin";
    
    char* reg_str = "^[A-Z][A-Za-z'-]+([ ][A-Z][A-Za-z'-]+)?*,[ ]?[A-Z][A-Za-z'-]+([ ][A-Z][A-Za-z'-]+)?*$";

    ret = regcomp(&reg, reg_str, cflags);
    if (ret)
    {   
        regerror(ret, &reg, ebuff, 256);
        fprintf(stderr, "%s\n", ebuff);
        goto end;
    }   

    ret = regexec(&reg, str, 0, NULL, 0);
    if (ret)
    {
        regerror(ret, &reg, ebuff, 256);
        fprintf(stderr, "invalid input syntax for type %s: \"%s\"", str);
        goto end;
    }   
        
    regerror(ret, &reg, ebuff, 256);
    fprintf(stderr, "result is:\n%s\n", ebuff);
    
end:
    regfree(&reg);

	// if (sscanf(str, " ( %lf , %lf )", &x, &y) != 2)
	// 	ereport(ERROR,
	// 			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
	// 			 errmsg("invalid input syntax for type %s: \"%s\"",
	// 					"complex", str)));

	result = (PersonName *) palloc(sizeof(PersonName));
	result->length = strlen(str);
	result->name = str;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pname_out);

Datum
pname_out(PG_FUNCTION_ARGS)
{
	PersonName    *str = (PersonName *) PG_GETARG_POINTER(0);
	char	   *result;

	result = psprintf("%s", str->name);
	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(pname_recv);

Datum
pname_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	PersonName  *result;

	result = (PersonName *) palloc(sizeof(PersonName));
	result->x = pq_getmsgcstring(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pname_send);

Datum
pname_send(PG_FUNCTION_ARGS)
{
	PersonName    *str = (PersonName *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendcstring(&buf, str->name);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Complex datatype would provide much more than this, of course.
 *****************************************************************************/



static int
pname_cmp(PersonName * a, PersonName * b)
{
	char *a_given, *b_given;
	int a_index, b_index;
	char *a_family, *b_family;
	int result;
	a_given = strchr(a->name, ',');
	b_given = strchr(b->name, ',');
	a_index = strlen(a->name) - strlen(a_given);
	b_index = strlen(b->name) - strlen(b_given);
	a_family = palloc((a_index+1) * sizeof(char));
	b_family = palloc((b_index+1) * sizeof(char));
	
	result = strcmp(a_family, b_family);
	if(result == 0){
		if(a->name[a_index+1] == ' ')
			a_given++;
		a_given++;
		if(b->name[b_index+1] == ' ')
			b_given++;
		b_given++;
		result = strcmp(a_given, b_given);
	}
	
	pfree(a_family);
	pfree(b_family);

	
	return result;
}


PG_FUNCTION_INFO_V1(pname_lt);

Datum
pname_lt(PG_FUNCTION_ARGS)
{
	PersonName   *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName   *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) < 0);
}

PG_FUNCTION_INFO_V1(pname_le);

Datum
pname_le(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(pname_eq);

Datum
pname_eq(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) == 0);
}

PG_FUNCTION_INFO_V1(pname_ge);

Datum
pname_ge(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(pname_gt);

Datum
pname_gt(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) > 0);
}

PG_FUNCTION_INFO_V1(pname_ne);

Datum
pname_ne(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	PersonName    *b = (PersonName *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(pname_cmp(a, b) != 0);
}

static char *
name_workshop(char * name, int mode)
{
  	char * given = strchr(name, ',');
  	char * family;
	char * result;
  	
  	printf("%s\n", str);
  	int index = strlen(name) - strlen(given);
//  	printf("%d\n", index);
	if(mode == 1){
  		result = palloc((index+1) * sizeof(char));
	
		strncpy(result, name, index);
		return result;
	}
  	// printf("%s\n", family);
//  	printf("%s\n", str);
//  	printf("%s\n", str);
	if(name[index+1] == ' ')
		given++;
	given++;
	if(mode == 2){
		result = given;
		return result;
	}
	family = palloc((index+1) * sizeof(char));
	strncpy(family, name, index);
	char * remain = strchr(given, ' ');
	char * first;
	index = strlen(given) - strlen(remain);
	first = palloc((index+1) * sizeof(char));
	strncpy(first, given, index);

	result = palloc((strlen(first) + strlen(family) +2) * sizeof(char));
	result = strcat(result, first);
	result = strcat(result, " ");
	result = strcat(result, family);

	
	
  	
//  	free(str);
//  	free(mid);
  	pfree(family);
	pfree(first);
    return result;
}


PG_FUNCTION_INFO_V1(family);

Datum
family(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	// char * result;
	PG_RETURN_CSTRING(name_workshop(a->name, 1));
}

PG_FUNCTION_INFO_V1(given);

Datum
given(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	// char * result;
	PG_RETURN_CSTRING(name_workshop(a->name, 2));
}

PG_FUNCTION_INFO_V1(show);

Datum
show(PG_FUNCTION_ARGS)
{
	PersonName    *a = (PersonName *) PG_GETARG_POINTER(0);
	// char * result;
	PG_RETURN_CSTRING(name_workshop(a->name, 3));
}
