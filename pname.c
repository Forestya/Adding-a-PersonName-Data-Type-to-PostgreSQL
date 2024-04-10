/*
 * src/tutorial/pname.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#include <regex.h>
#include "access/hash.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

typedef struct Pname
{
	int length;
	char name[FLEXIBLE_ARRAY_MEMBER];
} PersonName;

bool is_valid_person_name(char* str);

bool is_valid_person_name(char* str) {
    regex_t regex;
    int flag;
	char* comma = strchr(str, ',');

    flag = regcomp(&regex, "^[A-Z][A-Za-z'-]+([ ][A-Z][A-Za-z'-]+)*,[ ]?[A-Z][A-Za-z'-]+([ ][A-Z][A-Za-z'-]+)*$", REG_EXTENDED | REG_NOSUB);
    
	if (flag) {
        return false;
    }

	
	if (comma != NULL && *(comma + 1) == ' ' && *(comma + 2) != ' ') {
		memmove(comma + 1, comma + 2, strlen(comma + 2) + 1);
	}


    flag = regexec(&regex, str, 0, NULL, 0);
    

	if (!flag) {
        return true;
    } else {
        return false;
    }

	regfree(&regex);
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/
PG_FUNCTION_INFO_V1(pname_in);

Datum
pname_in(PG_FUNCTION_ARGS)
{
	char* str = PG_GETARG_CSTRING(0);
	PersonName* result;
	int length;

	if (!is_valid_person_name(str))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
						errmsg("invalid input syntax for type PersonName: \"%s\"", str)));

	length = strlen(str) + 1;
	result = (PersonName *) palloc(VARHDRSZ + length);
	SET_VARSIZE(result, VARHDRSZ + length);
	strncpy(result->name, str, length);

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(pname_out);

Datum
pname_out(PG_FUNCTION_ARGS)
{
	PersonName* pname = (PersonName*) PG_GETARG_POINTER(0);
	PG_RETURN_CSTRING(pname->name);
}

/*****************************************************************************
 * New Functions
 *****************************************************************************/
PG_FUNCTION_INFO_V1(pname_eq);

Datum
pname_eq(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) == 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}

PG_FUNCTION_INFO_V1(pname_lt);

Datum
pname_lt(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) < 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}

PG_FUNCTION_INFO_V1(pname_gt);

Datum
pname_gt(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) > 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}

PG_FUNCTION_INFO_V1(pname_ne);

Datum
pname_ne(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) != 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}

PG_FUNCTION_INFO_V1(pname_le);

Datum
pname_le(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) <= 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}

PG_FUNCTION_INFO_V1(pname_ge);

Datum
pname_ge(PG_FUNCTION_ARGS)
{
	PersonName* pname1 = (PersonName*) PG_GETARG_POINTER(0);
	PersonName* pname2 = (PersonName*) PG_GETARG_POINTER(1);
	
	if (strcmp(pname1->name, pname2->name) >= 0) {
		PG_RETURN_BOOL(true);
	} else {
		PG_RETURN_BOOL(false);
	}
}


PG_FUNCTION_INFO_V1(family);

Datum
family(PG_FUNCTION_ARGS)
{
	PersonName* pname = (PersonName*) PG_GETARG_POINTER(0);
	char* comma;
	char* family;
	int length;
	text* result;

	if (!is_valid_person_name(pname->name))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
						errmsg("invalid input syntax for type PersonName: \"%s\"", pname->name)));

	comma = strchr(pname->name, ',');

	length = comma - pname->name;
	family = (char*) palloc(length + 1);
	strncpy(family, pname->name, length);
	family[length] = '\0';

	result = (text*) palloc(VARHDRSZ + length + 1);
	SET_VARSIZE(result, VARHDRSZ + length);
	memcpy(VARDATA(result), family, length);

	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(given);

Datum
given(PG_FUNCTION_ARGS)
{
	PersonName* pname = (PersonName*) PG_GETARG_POINTER(0);
	char* comma;
	char* given;
	int length;
	text* result;

	if (!is_valid_person_name(pname->name))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
						errmsg("invalid input syntax for type PersonName: \"%s\"", pname->name)));

	comma = strchr(pname->name, ',');

	length = strlen(comma + 1);
	given = (char*) palloc(length + 1);
	strncpy(given, comma + 1, length);
	given[length] = '\0';

	result = (text*) palloc(VARHDRSZ + length + 1);
	SET_VARSIZE(result, VARHDRSZ + length);
	memcpy(VARDATA(result), given, length);

	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(show);

Datum
show(PG_FUNCTION_ARGS)
{
	PersonName* pname = (PersonName*) PG_GETARG_POINTER(0);
	char* comma;
	char* given;
	char* family;
	int length;
	text* result;
	char* trimmed_given;
	char* space;

	if (!is_valid_person_name(pname->name))
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
						errmsg("invalid input syntax for type PersonName: \"%s\"", pname->name)));

	comma = strchr(pname->name, ',');

	length = strlen(comma + 1);
	given = (char*) palloc(length + 1);
	strncpy(given, comma + 1, length);
	given[length] = '\0';

	// Find the first space in the given names
	space = strchr(given, ' ');
	if (space != NULL) {
		*space = '\0';  // Truncate the given names at the first space
	}

	trimmed_given = psprintf("%s", given);
	while (isspace((unsigned char) * trimmed_given)) {
		trimmed_given++;
	}

	length = comma - pname->name;
	family = (char*) palloc(length + 1);
	strncpy(family, pname->name, length);
	family[length] = '\0';

	result = (text*) palloc(VARHDRSZ + strlen(trimmed_given) + strlen(family) + 2);
	sprintf(VARDATA(result), "%s %s", trimmed_given, family);
	SET_VARSIZE(result, VARHDRSZ + strlen(VARDATA(result)));

	PG_RETURN_TEXT_P(result);
}


PG_FUNCTION_INFO_V1(pname_hash);

Datum
pname_hash(PG_FUNCTION_ARGS)
{
	PersonName *pname = (PersonName *) PG_GETARG_POINTER(0);
	
	char* temp_pname = pstrdup(pname->name);
	
	char* family_part = strtok(temp_pname, ",");
	char* given_part = strtok(NULL, ",");
	char *concat_name;
	int hash_code;

	if (given_part != NULL && *given_part == ' ') {
		given_part++;
	}
	
	concat_name = psprintf("%s %s", family_part, given_part);
	hash_code = DatumGetUInt32(hash_any((unsigned char *) concat_name, strlen(concat_name)));
	
	PG_RETURN_INT32(hash_code);
}
