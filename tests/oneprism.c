#include <silo.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define IND(i,j) i-1][j-1

#define matrix_assign(matrix,a11,a12,a13,a14,a21,a22,a23,a24,a31,a32,a33,a34,a41,a42,a43,a44)         \
   {                                                                          \
   matrix [IND(1,1)] = a11 ;                                              \
   matrix [IND(1,2)] = a12 ;                                              \
   matrix [IND(1,3)] = a13 ;                                              \
   matrix [IND(1,4)] = a14 ;                                              \
   matrix [IND(2,1)] = a21 ;                                              \
   matrix [IND(2,2)] = a22 ;                                              \
   matrix [IND(2,3)] = a23 ;                                              \
   matrix [IND(2,4)] = a24 ;                                              \
   matrix [IND(3,1)] = a31 ;                                              \
   matrix [IND(3,2)] = a32 ;                                              \
   matrix [IND(3,3)] = a33 ;                                              \
   matrix [IND(3,4)] = a34 ;                                              \
   matrix [IND(4,1)] = a41 ;                                              \
   matrix [IND(4,2)] = a42 ;                                              \
   matrix [IND(4,3)] = a43 ;                                              \
   matrix [IND(4,4)] = a44 ;                                              \
   }

#define matrix_mult(matrixa, matrixb, matrixc)                                \
   {                                                                          \
   for (i = 1; i < 5; i++) {                                                  \
      for (j = 1; j < 5; j++) {                                               \
         matrixc [IND(i,j)] = matrixa [IND(i,1)] * matrixb [IND(1,j)] + \
                                  matrixa [IND(i,2)] * matrixb [IND(2,j)] + \
                                  matrixa [IND(i,3)] * matrixb [IND(3,j)] + \
                                  matrixa [IND(i,4)] * matrixb [IND(4,j)] ; \
         }                                                                    \
      }                                                                       \
   }

#define RAD(deg)    M_PI*(deg/180.0)

/*-------------------------------------------------------------------------
 * Modifications:
 *    Hank Childs, Wed Feb 16 15:35:56 PST 2005
 *    Prism is inside out.  Correct it.
 *-------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    DBfile         *dbfile = NULL;
    char           *coordnames[3];
    float          *coords[3];
    int             nodelist[6];
    float           x[6], y[6], z[6];
    int             shapesize[1];
    int             shapecnt[1];
    DBfacelist     *facelist = NULL;
    int             matnos[1], matlist[1], dims[1];
    int             i, j, len;
    char            mesh_command[256];
    float           rot1[4][4], rot2[4][4], final[4][4];
    float           angle;
    float           var[6];
    int		    driver=DB_PDB;
    char	    *filename = "oneprism.silo";

    for (i=1; i<argc; i++) {
	if (!strcmp(argv[i], "DB_PDB")) {
	    driver = DB_PDB;
	    filename = "oneprism.pdb";
	} else if (!strcmp(argv[i], "DB_HDF5")) {
	    driver = DB_HDF5;
	    filename = "oneprism.h5";
	} else {
	    fprintf(stderr, "%s: ignored argument `%s'\n", argv[0], argv[i]);
	}
    }
    


    DBShowErrors(DB_ABORT, NULL);
    printf("Creating test file \"%s\".\n", filename);
    dbfile = DBCreate(filename, DB_CLOBBER, DB_LOCAL, "3D ucd prism", driver);

    coordnames[0] = "xcoords";
    coordnames[1] = "ycoords";
    coordnames[2] = "zcoords";

    /* This prism has the exact same shape and coordinates as zone 40 in 
     * globe.silo.  However, the variable stored across it is not the same as 
     * any globe.silo variables. */

    x[0] = 0.515; y[3] = 0; z[3] = 1.585;
    x[1] = 1.030; y[2] = 0; z[2] = 3.170;
    x[2] = 0; y[1] = 0; z[1] = 3.333;
    x[3] = 0; y[0] = 0; z[0] = 1.667;
    x[4] = 0.490; y[4] = 0.159; z[4] = 1.585;
    x[5] = 0.980; y[5] = 0.318; z[5] = 3.170;

    coords[0] = x;
    coords[1] = y;
    coords[2] = z;

    angle = 0;
    angle = M_PI*(angle/180.0);
    matrix_assign(rot1,
                  1, 0, 0, 0,
                  0, cos(angle), sin(angle), 0,
                  0, -sin(angle), cos(angle), 0,
                  0, 0, 0, 1);
    matrix_assign(rot2,
                  cos(angle), 0, -sin(angle), 0,
                  0, 1, 0, 0,
                  sin(angle), 0, cos(angle), 0,
                  0, 0, 0, 1);
    matrix_mult(rot1, rot2, final);

    for (i = 0; i < 6; i++)
    {
        float           tx, ty,tz;

        tx = x[i]*final[IND(1,1)] + y[i]*final[IND(1,2)] + z[i]*final[IND(1,3)] + final[IND(1,4)];
        ty = x[i]*final[IND(2,1)] + y[i]*final[IND(2,2)] + z[i]*final[IND(2,3)] + final[IND(2,4)];
        tz = x[i]*final[IND(3,1)] + y[i]*final[IND(3,2)] + z[i]*final[IND(3,3)] + final[IND(3,4)];

        x[i] = tx;
        y[i] = ty;
        z[i] = tz;

        var[i] = x[i]+y[i]*z[i];
    }

    DBPutUcdmesh(dbfile, "prism", 3, coordnames, coords, 6, 1, "zonelist",
                 "facelist", DB_FLOAT, NULL);

    matnos[0] = 1;
    matlist[0] = 1;
    dims[0] = 1;

    DBPutMaterial(dbfile, "mat", "prism", 1, matnos, matlist, dims,
                  1, NULL, NULL, NULL, NULL, 0, DB_FLOAT, NULL);

    DBPutUcdvar1(dbfile, "v", "prism", var, 6, NULL, 0, DB_FLOAT, DB_NODECENT,
                 NULL);

    nodelist[0] = 0;
    nodelist[1] = 1;
    nodelist[2] = 2;
    nodelist[3] = 3;
    nodelist[4] = 4;
    nodelist[5] = 5;

    shapesize[0] = 6;
    shapecnt[0] = 1;

    DBSetDeprecateWarnings(0);
    DBPutZonelist(dbfile, "zonelist", 1, 3, nodelist, 6, 0, shapesize,
                  shapecnt, 1);
    DBSetDeprecateWarnings(3);

    facelist = DBCalcExternalFacelist(nodelist, 6, 0, shapesize, shapecnt, 1,
                                      NULL, 0);

    DBPutFacelist(dbfile, "facelist", facelist->nfaces, facelist->ndims,
                  facelist->nodelist, facelist->lnodelist, facelist->origin,
                  facelist->zoneno, facelist->shapesize, facelist->shapecnt,
                  facelist->nshapes, facelist->types, facelist->typelist,
                  facelist->ntypes);

    sprintf(mesh_command, "mesh prism; contour v");
    len = strlen(mesh_command) + 1;
    DBWrite(dbfile, "_meshtvinfo", mesh_command, &len, 1, DB_CHAR);

    DBClose(dbfile);

    return (0);
}
