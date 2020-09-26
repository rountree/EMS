/**
 *
 *  #### ENVIRONMENTAL MODELLING SUITE (EMS)
 *
 *  \file lib/io/tf_nc_cmr_tiled.c
 *
 *  \brief Functions to access tiled CMR style netCDF bathy/topo files
 *
 *  \copyright
 *  Copyright (c) 2018. Commonwealth Scientific and Industrial
 *  Research Organisation (CSIRO). ABN 41 687 119 230. All rights
 *  reserved. See the license file for disclaimer and full
 *  use/redistribution conditions.
 *
 *  $Id: tf_nc_cmr_tiled.c 5833 2018-06-27 00:21:35Z riz008 $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h>
#include "netcdf.h"
#include "ems.h"

/* external prototypes */
int get_lat_index(topo_details_t *td, double lat);
int get_lon_index(topo_details_t *td, double lon);

/* local functions */
/**
 * check to see if file is a valid tiled nc file
 */
int tf_nc_cmr_tiled_is_valid(char *fname) {
   //int is_valid = 0;	/* invalid by default */
   FILE *ttfp, *testfp;
   char keyword[MAXSTRLEN];
   char vers[MAXSTRLEN];
   char basedir[MAXSTRLEN];
   char colnames[MAXSTRLEN];
   char *colvals[MAXSTRLEN];
   int nrows = 0;
   int ncols = 0;
   int numread = 0;
   int i, j;
   int ncid, lonVid, latVid, htVid;

   ttfp = fopen(fname, "r");

   if(ttfp == NULL){ 
     // FIXME Add errno reporting.
     warn("File '%s' does not exist.", fname);
     return 0;
   }
   if(0 == prm_read_char(ttfp, "VERSION", vers)){
     // VERSION keyword missing
     warn("File '%s' does not contain a VERSION keyword.", fname);
     return 0;
   }
   if(0 != strcmp("nctiled", vers)){
     // Wrong version
     warn("File '%s' version is not 'nctiled'.", fname);
     return 0;
   }
   if(0 == prm_read_char(ttfp, "BASEDIR", basedir)){
     // Getting BASEDIR failed
     warn("File '%s' does not have a BASEDIR keyword.", fname);
     return 0;
   }

   testfp = fopen(basedir, "r");
   if( NULL == testfp ){
     warn("Could not resolve '%s', trying a different BASEDIR.", basedir);
     char *oldbasedir = malloc( strlen(basedir)+1 );
     char *oldfname = malloc( strlen(fname)+1 );
     if( !oldbasedir || !oldfname ){
	     warn("malloc() failed.  Out of memroy.");
	     free(oldbasedir);
	     free(oldfname);
	     return 0;
     }
     snprintf(oldbasedir, strlen(basedir)+1, "%s", basedir);
     snprintf(oldfname, strlen(fname)+1, "%s", fname);
     char *bdir = dirname(oldfname);
     if( strlen(bdir) + strlen(oldbasedir) + 2 > MAXSTRLEN ){
       warn("%s/%s is longer than MAXSTRLEN=%d.\n", bdir, oldbasedir, MAXSTRLEN);
       return 0;
     }
     snprintf( basedir, strlen(bdir)+strlen(oldbasedir)+2, "%s/%s", bdir, oldbasedir );
     free(oldbasedir);
     free(oldfname);
   }else{
     fclose(testfp);
   }

   testfp = fopen( basedir, "r" );
   if( NULL==testfp ){
     warn("Could not resolve '%s', giving up.", basedir);
     return 0;
   }else{
     fclose(testfp);
   }

   if( 0==prm_read_int(ttfp, "NROWS", &nrows )){
     warn("Could not access NROWS.");
     return 0;
   }

   if( 0==prm_read_int(ttfp, "NCOLS", &ncols )){
     warn("Could not access NCOLS.");
     return 0;
   }

   /* Read and check the file list */
   for( i=1; i<ncols; ++i ){
     snprintf( keyword, MAXSTRLEN, "COL%i", i);
     if( 0 == prm_read_char( ttfp, keyword, colnames )){
       warn("Unable to find keyword '%s'", keyword);
       return 0;
     }
     numread = parseline(colnames, colvals, nrows);
     for(j=0;j<numread;++j){
       char *colval_filename = malloc( strlen(basedir) + strlen(colvals[j]) + 2 );
       if( !colval_filename ){
	       warn("Malloc failed, out of memory.");
	       return 0;
       }
       snprintf(colval_filename, strlen(basedir)+strlen(colvals[j])+2, "%s/%s", basedir, colvals[j]);
       if( NC_NOERR != nc_open(keyword, NC_NOWRITE, &ncid) ){
         warn("Failure opening '%s'.", colval_filename);
         free( colval_filename );
         return 0;
       }

       if( NC_NOERR != nc_inq_varid( ncid, "lat", &latVid ) ){
	 warn("'lat' test failed for %s.", colval_filename);
	 free( colval_filename );
	 return 0;
       }

       if( NC_NOERR != nc_inq_varid( ncid, "lon", &lonVid) ){
	 warn("'lon' test failed for %s.", colval_filename);
	 free( colval_filename );
	 return 0;
       }

       if( ( NC_NOERR != nc_inq_varid( ncid, "height", &htVid ) )
       &&  ( NC_NOERR != nc_inq_varid( ncid, "depth", &htVid ) ) ){
	 warn("'height'/'depth' test failed for %s.", colval_filename);
	 free( colval_filename);
	 return 0;
       }
       free( colval_filename);
     } // for j 
   } // for i
   return 1;
}

topo_details_t *tf_nc_cmr_tiled_open(char *fname) {
   topo_details_t *td;
   FILE *ttfp;
   char keyword[MAXSTRLEN];
   char basedir[MAXSTRLEN];
   char *valid_basedir=NULL;
   int nrows = 0;
   int ncols = 0;

  td = (topo_details_t*)malloc(sizeof(topo_details_t));
  memset(td, 0, sizeof(topo_details_t));
  td->extent = (topo_extent_t*)malloc(sizeof(topo_extent_t));  
  memset(td->extent, 0, sizeof(topo_extent_t));

   ttfp = fopen(fname, "r");
   strcpy(td->name, fname);
   td->fp = ttfp;
   td->topo_type = GRID_POINT;
   strcpy(keyword, "BASEDIR");
   (void)prm_read_char(ttfp, keyword, basedir);
   /* Now validate the base dir */
   if (fopen(basedir, "r") == NULL) {
     char oldbasedir[MAXSTRLEN];
     char oldfname[MAXSTRLEN];
     sprintf(oldbasedir, "%s", basedir);
     sprintf(oldfname  , "%s", fname);
     char *bdir = dirname(fname);
     /* Prepend the basedir of the bathy file */
     valid_basedir=malloc(strlen(bdir)+strlen(oldbasedir)+2);
     snprintf(valid_basedir, strlen(bdir)+strlen(oldbasedir)+2, "%s/%s", bdir, oldbasedir);
   }else{
     valid_basedir=basedir;
   }
   strcpy(td->basedir, valid_basedir);
   free(valid_basedir);
   strcpy(keyword, "NROWS");
   (void)prm_read_int(ttfp, keyword, &nrows);
   td->num_file_rows = nrows;
   strcpy(keyword, "NCOLS");
   (void)prm_read_int(ttfp, keyword, &ncols);
   td->num_file_cols = ncols;
   /* set the number of file tiles */
   td->num_file_tiles = nrows * ncols;

   /* set the unrelated values to "non values" */
   td->nlats = -1;
   td->nlons = -1;
   td->delx = -1;
   td->dely = -1;
   td->lats = NULL;
   td->lons = NULL;
   td->z = NULL;
   td->stats = NULL;
   td->ncid = -1;

   return(td);
} /* tf_nc_cmr_tiled_open */

int tf_nc_cmr_tiled_close(topo_t *tf) {
   int close_ok = 0;
   int i;

   if(!fclose(tf->td->fp)) {
      /* fclose rets 0 if close OK */
      close_ok = 1;
   }
   for(i=0;i<tf->tts.ntiles;++i) {
      if(nc_close(tf->tts.tiles[i]->td->ncid) != NC_NOERR) {
         close_ok = 0;
      }
   }

   return(close_ok);
} /* tf_nc_cmr_tiled_close */

int tf_nc_cmr_tiled_getz(topo_details_t *td, double *x, double *y, double *z, 
   int nx, int ny) {
   int got_ok = 0;
   int htvid = -1;
   size_t start[2];
   size_t count[2];
   int i, j, k;
  double make_topo = -1.0;
   double **zz;
   int err;

  if(nc_inq_varid(td->ncid, "height", &htvid) == NC_NOERR)
    make_topo = 1.0;
  else {
    /* make bathy into topo */
    nc_inq_varid(td->ncid, "depth", &htvid);
    make_topo = -1.0;
  }

   start[0] = get_lat_index(td, y[0]);
   start[1] = get_lon_index(td, x[0]);

/*UR-TODO what is the point of checking an unsigned int/long for being smaller 0? */
  if(start[0] < 0) {
    warn("tf_nc_cmr_tiled_getz: lat index not found %.2f in %s\n", y[0],
      td->name);
    return(0);
  }
/*UR-TODO what is the point of checking an unsigned int/long for being smaller 0? */
  if(start[1] < 0) {
    warn("tf_nc_cmr_tiled_getz: lon index not found %.2f in %s\n", x[0],
      td->name);
    return(0);
  }

   /* set the lon count */
   count[0] = nx;
   /* set the lat count */
   count[1] = ny;
   /* allocate the zz array */
   zz = d_alloc_2d(nx,ny);
   err = nc_get_vara_double(td->ncid, htvid, start, count, zz[0]);
   if(err != NC_NOERR) {
      warn("Can't get depth from file %s : %d %d : %d %d\n", td->name, start[0], start[1], count[0], count[1]);
      got_ok = 0;
   } else {
      k = 0;
      for(i=0;i<ny;++i) {
         for(j=0;j<nx;++j) {
            /* NOTE: zz[y][x] */
            z[k] = zz[i][j] * make_topo;
            ++k;
         }
      }
      got_ok = 1;
   }/* end if err != NC_NOERR */
   d_free_2d(zz);

   return(got_ok);
} /* end tf_nc_cmr_tiled_getz */

topo_extent_t *tf_nc_cmr_tiled_extent(topo_details_t *td) {
   topo_extent_t *te;
   double minlat[2], minlon[2], maxlat[2], maxlon[2];
   size_t nlat, nlon;
   size_t start[2], end[2];
   char keyword[MAXSTRLEN];
   char basedir[MAXSTRLEN];
   char colnames[MAXSTRLEN];
   char *colvals[MAXSTRLEN];
   int nrows = 0;
   int ncols = 0;
   int ncid, lonDid, lonVid, latDid, latVid;
   char *nc_filename=NULL;

  te = (topo_extent_t*)malloc(sizeof(topo_extent_t));  
  memset(te, 0, sizeof(topo_extent_t));

   strcpy(basedir, td->basedir);
   nrows = td->num_file_rows;
   ncols = td->num_file_cols;
   sprintf(keyword, "COL%i", 1);
   (void)prm_read_char(td->fp, keyword, colnames);
   (void)parseline(colnames, colvals, nrows);
   /* 1st column 1st row */
   nc_filename=malloc( strlen(basedir) + strlen(colvals[0]) + 2 );
   snprintf(nc_filename, strlen(basedir) + strlen(colvals[0]) + 2, "%s/%s", basedir, colvals[0]);
   nc_open(nc_filename, NC_NOWRITE, &ncid);
   free(nc_filename);
   nc_inq_varid(ncid, "lon", &lonVid);
   start[0] = 0;
   end[0] = 2;
   nc_get_vara_double(ncid, lonVid, start, end, minlon);
   nc_inq_varid(ncid, "lat", &latVid);
   start[0] = 0;
   end[0] = 2;
   nc_get_vara_double(ncid, latVid, start, end, minlat);
   nc_close(ncid);
   sprintf(keyword, "COL%i", ncols);
   (void)prm_read_char(td->fp, keyword, colnames);
   (void)parseline(colnames, colvals, nrows);
   /* last column last row */
   nc_filename=malloc( strlen(basedir) + strlen(colvals[nrows-1]) + 2 );
   snprintf(keyword, strlen(basedir) + strlen(colvals[nrows-1]) + 2, "%s/%s", basedir, colvals[nrows-1]);
   nc_open(keyword, NC_NOWRITE, &ncid);
   free(nc_filename);
   nc_inq_dimid(ncid, "lon", &lonDid);
   nc_inq_dimlen(ncid, lonDid, &nlon);
   nc_inq_varid(ncid, "lon", &lonVid);
   start[0] = (nlon-2);
   end[0] = 2;
   nc_get_vara_double(ncid, lonVid, start, end, maxlon);
   nc_inq_dimid(ncid, "lat", &latDid);
   nc_inq_dimlen(ncid, latDid, &nlat);
   nc_inq_varid(ncid, "lat", &latVid);
   start[0] = (nlat-2);
   end[0] = 2;
   nc_get_vara_double(ncid, latVid, start, end, maxlat);
   nc_close(ncid);

  if(td->topo_type == GRID_POLY) {
    te->minLat = minlat[0];
    te->maxLat = maxlat[1];
    te->minLon = minlon[0];
    te->maxLon = maxlon[1];
  } else if(td->topo_type == GRID_POINT) {
    te->minLat = minlat[0] - fabs(minlat[1] - minlat[0]) / 2.0;
    te->maxLat = maxlat[1] + fabs(maxlat[1] - maxlat[0]) / 2.0;
    te->minLon = minlon[0] - fabs(minlon[1] - minlon[0]) / 2.0;
    te->maxLon = maxlon[1] + fabs(minlon[1] - minlon[0]) / 2.0;
  } else
    quit("tf_nc_cmr_tiled_extent: unsupported topo_type\n");

   return(te);
} /* end tf_nc_cmr_tiled_extent */

topo_tile_t *tf_nc_cmr_tiled_get_tile(topo_t *tf, int col, int row) {
   topo_tile_t *tile;
   int ncid, lonDid, lonVid, latDid, latVid;
   char keyword[MAXSTRLEN];
   char colnames[MAXSTRLEN];
   char *colvals[MAXSTRLEN];
   /*UR-CHANGED comply with ia64 */
   size_t nlat, nlon;
   size_t start[2], end[2];
   char *nc_filename=NULL;

  tile = (topo_tile_t*)malloc(sizeof(topo_tile_t));
  memset(tile, 0, sizeof(topo_tile_t));
  tile->td = (topo_details_t*)malloc(sizeof(topo_details_t));
  memset(tile->td, 0, sizeof(topo_details_t));
  tile->td->extent = (topo_extent_t*)malloc(sizeof(topo_extent_t));  
  memset(tile->td->extent, 0, sizeof(topo_extent_t));
  tile->neighbours = (topo_neighbours_t*)malloc(sizeof(topo_neighbours_t));
  memset(tile->neighbours, 0, sizeof(topo_neighbours_t)); 

  /* inherit the topo_type from the parent file */
  tile->td->topo_type = tf->td->topo_type;

   /* internal tile numbering is 0 to ncols-1, tiled topo is
    * 1 to ncols so adjust for that */
   sprintf(keyword, "COL%i", col+1);
   (void)prm_read_char(tf->td->fp, keyword, colnames);
   (void)parseline(colnames, colvals, tf->td->num_file_rows);
   nc_filename = malloc( strlen( tf->td->basedir ) + strlen( colvals[row] ) + 2 );
   snprintf(nc_filename, strlen( tf->td->basedir ) + strlen( colvals[row] ) + 2, "%s/%s", tf->td->basedir, colvals[row]);
   strcpy(tile->td->name, nc_filename);
   if(nc_open(nc_filename, NC_NOWRITE, &ncid) != NC_NOERR){
     quit("tf_nc_cmr_tiled_get_tile: error opening file: \"%s\"\n", tile->td->name);
   }
   free(nc_filename);
   tile->td->ncid = ncid;
   nc_inq_dimid(ncid, "lon", &lonDid);
   nc_inq_dimlen(ncid, lonDid, &nlon);
   tile->td->nlons = nlon;
   nc_inq_varid(ncid, "lon", &lonVid);
   start[0] = 0;
   end[0] = nlon;
   tile->td->lons = d_alloc_1d(nlon);
   nc_get_vara_double(ncid, lonVid, start, end, tile->td->lons);
   tile->td->delx = fabs(tile->td->lons[1] - tile->td->lons[0]);
  if(tile->td->topo_type == GRID_POINT) {
    tile->td->extent->minLon = tile->td->lons[0] - (tile->td->delx / 2.0);
    tile->td->extent->maxLon = tile->td->lons[nlon-1] + (tile->td->delx / 2.0);
  } else if(tile->td->topo_type == GRID_POLY) {
    tile->td->extent->minLon = tile->td->lons[0];
    tile->td->extent->maxLon = tile->td->lons[nlon-1];
  } else
    quit("tf_nc_cmr_tiled_get_tile: unsupported topo_type\n");

   nc_inq_dimid(ncid, "lat", &latDid);
   nc_inq_dimlen(ncid, latDid, &nlat);
   tile->td->nlats = nlat;
   nc_inq_varid(ncid, "lat", &latVid);
   start[0] = 0;
   end[0] = nlat;
   tile->td->lats = d_alloc_1d(nlat);
   nc_get_vara_double(ncid, latVid, start, end, tile->td->lats);
   tile->td->dely = fabs(tile->td->lats[1] - tile->td->lats[0]);
  if(tile->td->topo_type == GRID_POINT) {
    tile->td->extent->minLat = tile->td->lats[0] - (tile->td->dely / 2.0);
    tile->td->extent->maxLat = tile->td->lats[nlat-1] + (tile->td->dely / 2.0);
  } else if(tile->td->topo_type == GRID_POLY) {
    tile->td->extent->minLat = tile->td->lats[0];
    tile->td->extent->maxLat = tile->td->lats[nlat-1];
  } else
    quit("tf_nc_cmr_tiled_get_tile: unsupported topo_type\n");

   /* set the neighbours
    *
    *      -1      +1
    *     +---+---+---+     x = tile being processed
    *  +1 |   |   |   |
    *     +---+---+---+
    *     |   | X |   |
    *     +---+---+---+
    *  -1 |   |   |   |
    *     +---+---+---+
    */
   /* 1st set all to zero */
   tile->neighbours->topLeft = 0;
   tile->neighbours->top = 0;
   tile->neighbours->topRight = 0;
   tile->neighbours->left = 0;
   tile->neighbours->right = 0;
   tile->neighbours->botLeft = 0;
   tile->neighbours->bot = 0;
   tile->neighbours->botRight = 0;
   /* now calc the columns */
   if(col == 0) {
      tile->neighbours->topLeft = -1;
      tile->neighbours->left = -1;
      tile->neighbours->botLeft = -1;
      tile->neighbours->right = (col + 1) * tf->td->num_file_rows + row;
   } else if(col == (tf->td->num_file_cols-1)) {
      tile->neighbours->topRight = -1;
      tile->neighbours->right = -1;
      tile->neighbours->botRight = -1;
      tile->neighbours->left = (col - 1) * tf->td->num_file_rows + row;
   } else {
      /* calc tile number */
      tile->neighbours->left = (col - 1) * tf->td->num_file_rows + row;
      tile->neighbours->right = (col + 1) * tf->td->num_file_rows + row ;
   }
   /* next the rows */
   if(row == 0) {
      tile->neighbours->botLeft = -1;
      tile->neighbours->bot = -1;
      tile->neighbours->botRight = -1;
      tile->neighbours->top = col * tf->td->num_file_rows + (row + 1);
   } else if(row == (tf->td->num_file_rows-1)) {
      tile->neighbours->topLeft = -1;
      tile->neighbours->top = -1;
      tile->neighbours->topRight = -1;
      tile->neighbours->bot = col * tf->td->num_file_rows + (row - 1);
   } else {
      /* calc tile number */
      tile->neighbours->top = col * tf->td->num_file_rows + (row + 1);
      tile->neighbours->bot = col * tf->td->num_file_rows + (row - 1);
   }
   /* now the corners */
   if(tile->neighbours->botLeft == 0) {
      tile->neighbours->botLeft = tile->neighbours->left - 1;
   }
   if(tile->neighbours->botRight == 0) {
      tile->neighbours->botRight = tile->neighbours->right - 1;
   }
   if(tile->neighbours->topLeft == 0) {
      tile->neighbours->topLeft = tile->neighbours->left + 1;
   }
   if(tile->neighbours->topRight == 0) {
      tile->neighbours->topRight = tile->neighbours->right + 1;
   }

   return(tile);
} /* end tf_nc_cmr_tiled_get_tile */
