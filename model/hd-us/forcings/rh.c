/*
 *
 *  ENVIRONMENTAL MODELLING SUITE (EMS)
 *  
 *  File: model/hd-us/forcings/rh.c
 *  
 *  Description:
 *  Event scheduler routines for Atmospheric rh.
 *  
 *  Copyright:
 *  Copyright (c) 2018. Commonwealth Scientific and Industrial
 *  Research Organisation (CSIRO). ABN 41 687 119 230. All rights
 *  reserved. See the license file for disclaimer and full
 *  use/redistribution conditions.
 *  
 *  $Id: rh.c 5873 2018-07-06 07:23:48Z riz008 $
 *
 */

#include <math.h>
#include <string.h>
#include "hd.h"

typedef struct {
  double dt;                    /* Time step */
  timeseries_t *ts;             /* Timeseries file */
  int id;                       /* TS id for the variable */
} rh_data_t;

/* Functions for reading the schedule the rh forcings. */
int rh_init(sched_event_t *event)
{
  master_t *master = (master_t *)schedGetPublicData(event);
  parameters_t *params = master->params;
  rh_data_t *data = NULL;

  /* Read parameters */
  prm_set_errfn(hd_silent_warn);

  data = (rh_data_t *)malloc(sizeof(rh_data_t));
  schedSetPrivateData(event, data);
  if ((data->ts = frcw_read_cell_ts(master, params->rh, params->rh_dt,
                              "humidity", "%", &data->dt, &data->id,
                              &master->rh)) == NULL)
    data->ts = frc_read_cell_ts(master, params->rh, params->rh_dt,
				"humidity", "percent", &data->dt, &data->id,
				&master->rh);

  if (data->ts == NULL) {
    free(data);
    return 0;
  }
  if (master->sh_f & NONE) master->sh_f = RELHUM;

  return 1;
}

double rh_event(sched_event_t *event, double t)
{
  master_t *master = (master_t *)schedGetPublicData(event);
  rh_data_t *data = (rh_data_t *)schedGetPrivateData(event);

  if (t >= (event->next_event - SEPS)) {
    /* Humidity, convert to fraction (0-1) */
    frc_ts_eval_grid(master, t, data->ts, data->id, master->rh, 0.01);
    event->next_event += data->dt;
  }

  return event->next_event;
}

void rh_cleanup(sched_event_t *event, double t)
{
  rh_data_t *data = (rh_data_t *)schedGetPrivateData(event);

  if (data != NULL) {
    free(master->rh);
    hd_ts_free(master, data->ts);
    free(data);
  }
}
