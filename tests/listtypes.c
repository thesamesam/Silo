/*

                           Copyright 1991 - 2000
                The Regents of the University of California.
                            All rights reserved.

This work was produced at the University of California, Lawrence
Livermore National Laboratory (UC LLNL) under contract no.  W-7405-ENG-48
(Contract 48) between the U.S. Department of Energy (DOE) and The Regents
of the University of California (University) for the operation of UC LLNL.
Copyright is reserved to the University for purposes of controlled
dissemination, commercialization through formal licensing, or other
disposition under terms of Contract 48; DOE policies, regulations and
orders; and U.S. statutes.  The rights of the Federal Government are
reserved under Contract 48 subject to the restrictions agreed upon by
DOE and University.

                                DISCLAIMER

This software was prepared as an account of work sponsored by an agency
of the United States Government. Neither the United States Government
nor the University of California nor any of their employees, makes any
warranty, express or implied, or assumes any liability or responsiblity
for the accuracy, completeness, or usefullness of any information,
apparatus, product, or process disclosed, or represents that its use
would not infringe privately owned rights. Reference herein to any
specific commercial products, process, or service by trade name, trademark,
manufacturer, or otherwise, does not necessarily constitute or imply its
endorsement, recommendation, or favoring by the United States Government
or the University of California. The views and opinions of authors
expressed herein do not necessarily state or reflect those of the United
States Government or the University of California, and shall not be used
for advertising or product endorsement purposes.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <silo.h>

#define ALLOC_N(T, N) (T *)malloc((N) * sizeof(T))
#define FREE(P) if(P) {free(P); (P) = NULL;}

/* Prototypes */
void PrintFileComponentTypes(char *);
void PrintObjectComponentsType(DBfile *, char *, char *);

/*********************************************************************
 *
 * Purpose: Converts an int DB type to its string name.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 14:12:23 PST 2000
 *
 * Input Arguments:
 *    type : This is an integer representing a DB type.
 *
 * Modifications:
 *
 ********************************************************************/

char *
IntToTypename(int type)
{
    char *retval;

    switch(type)
    {
    case DB_INT:
        retval = "DB_INT";
        break;
    case DB_SHORT:
        retval = "DB_SHORT";
        break;
    case DB_LONG:
        retval = "DB_LONG";
        break;
    case DB_FLOAT:
        retval = "DB_FLOAT";
        break;
    case DB_DOUBLE:
        retval = "DB_DOUBLE";
        break;
    case DB_CHAR:
        retval = "DB_CHAR";
        break;
    case DB_VARIABLE:
        retval = "DB_VARIABLE";
        break;
    default:
        retval = "DB_NOTYPE";
    }

    return retval;
}

/*********************************************************************
 *
 * Purpose: Main function for listtypes.c. This function iterates
 *          over the command line arguments and supplies them to
 *          a function that prints the component types for a file.
 *          This program tests the DBGetComponentType function.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:05:37 PST 2000
 *
 * Input Arguments:
 *     argc : The number of command line arguments.
 *     argv : An array containing the command line arguments.
 *
 * Modifications:
 *     Thomas R. Treadway, Thu Jul  5 16:33:38 PDT 2007
 *     Chaneged main's return type to int, to stop gcc-4.x whining.
 *
 ********************************************************************/

int
main(int argc, char *argv[])
{
    int i;

    if(argc < 2)
    {
        printf("Usage: listtypes filename [filename ...]\n");
        exit(0);
    }

    /* Print the types for components in the specified files. */
    for(i = 1; i < argc; i++)
        PrintFileComponentTypes(argv[i]);
    
    return 0;
}

/*********************************************************************
 *
 * Purpose: Macroize code that processes entries in the TOC.
 *
 * The last 'S' argument to the macro is used to handle plural or
 * singular form of specification of toc data members.
 *
 * Programmer: Mark C. Miller
 * Date:       June 19, 2008 
 *
 ********************************************************************/
#define PRINT_OBJS(theFile, theToc, theClass, Indent, S)        \
    nobjs += theToc->n ## theClass ## S;                                \
    for (i = 0; i < theToc->n ## theClass ## S; i++)            \
        PrintObjectComponentsType(theFile, theToc->theClass ## _names[i], Indent);

int ProcessCurrentDirectory(DBfile *dbfile, DBtoc *dbtoc, int depth)
{
    int i, j, nobjs;
    char indent[1024];
    int ndirs = dbtoc->ndir;

    /* compute an indent */
    for (i = 0; i < depth * 3; i++)
        indent[i] = ' ';
    indent[i] = '\0';

    /* descend into subdirs, first */
    if(ndirs > 0)
    {
        /* Make a list of all the directory names since the list
         * in the TOC will change as we change directories.
         */
        char currentdir[1024];
        char **dir_names = ALLOC_N(char *, ndirs);
        DBGetDir(dbfile, currentdir);
        for(i = 0; i < ndirs; i++)
        {
            dir_names[i] = ALLOC_N(char, 1+strlen(dbtoc->dir_names[i]+
                                   strlen(currentdir)));
            sprintf(dir_names[i], "%s%s", currentdir, dbtoc->dir_names[i]);
        }

        /* Search each directory for objects. */
        for(j = 0; j < ndirs; j++)
        {
            /* Change directories and get the TOC. */
            DBtoc *current_dbtoc;
            DBSetDir(dbfile, dir_names[j]);
            current_dbtoc = DBGetToc(dbfile);
            printf("%sDirectory: %s\n", indent, dir_names[j]);
            if (ProcessCurrentDirectory(dbfile, current_dbtoc, depth+1) <= 0)
                printf("%s<directory contains no objects>\n\n", indent);
        }

        /* Free the directory list. */
        for(i = 0; i < ndirs; i++)
            FREE(dir_names[i]);
        FREE(dir_names);
    }

    /* Print the objects in the top directory. */
    nobjs = ndirs; 
    PRINT_OBJS(dbfile, dbtoc, obj, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, defvars, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, array, indent, s);
    PRINT_OBJS(dbfile, dbtoc, curve, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, ptmesh, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, ptvar, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, qmesh, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, qvar, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, ucdmesh, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, ucdvar, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, csgmesh, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, csgvar, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, mat, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, matspecies, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, multimesh, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, multimeshadj, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, multivar, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, multimat, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, multimatspecies, indent, /*void*/);
    PRINT_OBJS(dbfile, dbtoc, mrgtree, indent, s);
    PRINT_OBJS(dbfile, dbtoc, mrgvar, indent, s);
    PRINT_OBJS(dbfile, dbtoc, groupelmap, indent, s);
    return nobjs;
}

/*********************************************************************
 *
 * Purpose: Reads the specified file and prints out the types of all
 *          of its components.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 12:21:36 PDT 2000
 *
 * Input Arguments:
 *     filename : The path and name the file we want to print out.
 *
 * Modifications:
 *
 ********************************************************************/

void
PrintFileComponentTypes(char *filename)
{
    int    i, j, ndirs, nobj = 0;
    char   topdir[200], **dir_names = NULL;
    DBfile *dbfile = NULL;
    DBtoc  *dbtoc = NULL;

    /* Open the data file. Return if it cannot be read. */
    if((dbfile = DBOpen(filename, DB_UNKNOWN, DB_READ)) == NULL)
    {
        printf("File: %s\n    <could not be opened>\n\n", filename);
        return;
    }

    /* Read the file's table of contents. */
    if((dbtoc = DBGetToc(dbfile)) == NULL)
    {
        printf("File: %s\n    <could not read TOC>\n\n", filename);
        DBClose(dbfile);
        return;
    }

    printf("File: %s\n", filename);

    if (ProcessCurrentDirectory(dbfile, dbtoc, 0) <= 0)
        printf("<file contains no objects>\n\n");

    /* Close the file. */
    DBClose(dbfile);
}

/*********************************************************************
 *
 * Purpose: Prints all of the components in the specified object.
 *          The component's name, type, and value are printed. The
 *          value is printed using the type information gathered
 *          from DBGetComponentType.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 20 13:09:48 PST 2000
 *
 * Input Arguments:
 *     dbfile  : The database file.
 *     objname : The name of the object whose components we're
 *               going to print.
 *
 * Modifications:
 *
 ********************************************************************/

void
PrintObjectComponentsType(DBfile *dbfile, char *objname, char *indent)
{
    int  i, comptype = DB_NOTYPE;
    DBobject *obj = NULL;

    /* Get the component names for the object. */
    if((obj = DBGetObject(dbfile, objname)) == NULL)
    {
        printf("%sObject: \"%s\"\n    %s<cannot read object>\n\n", indent,
               objname, indent);
        return;
    }

    printf("%sObject: \"%s\"\n", indent, objname);
    if(obj->ncomponents > 0)
    {
        void *comp = NULL;

        /* For each component, read its type and print it. */
        for(i = 0; i < obj->ncomponents; i++)
        {
            comptype = DBGetComponentType(dbfile, objname, obj->comp_names[i]);

            comp = NULL;
            if(comptype != DB_VARIABLE)
                comp = DBGetComponent(dbfile, objname, obj->comp_names[i]);

            printf("    %sComponent: %-15s  Type: %-11s",
                   indent, obj->comp_names[i], IntToTypename(comptype));

            if(comp != NULL)
            {
                /* Use the type information returned by DBGetComponentType 
                 * to correctly process the component data.
                 */
                switch(comptype)
                {
                case DB_INT:
                    printf(" Value: %d\n", *((int *)comp));
                    break;
                case DB_CHAR:
                    printf(" Value: %s\n", (char *)comp);
                    break;
                case DB_FLOAT:
                    printf(" Value: %g\n", *((float *)comp));
                    break;
                case DB_DOUBLE:
                    printf(" Value: %.30g\n", *((double *)comp));
                    break;
                default:
                    printf(" Value: ???\n");
                }

                /* Free the component memory. */
                free(comp);
            }
            else
                printf("\n");
        }
        printf("\n");
    }
    else
        printf("    %s<no components>\n\n", indent);

    /* Free the object.*/
    DBFreeObject(obj);
}
