/*
 *
 *  ENVIRONMENTAL MODELLING SUITE (EMS)
 *  
 *  File: model/lib/ecology/process_library/phytoplankton_spectral_mortality_sed.c
 *  
 *  Description: Phytoplankton mortality in sediments
 *
 *  Options: phytoplankton_spectral_mortality_wc(small|large)
 *
 *  Small - State variables PhyS_*, parameters PS*
 *  Large - State variables PhyL_*, parameters PL*
 *
 *  Process includes return of nutrient reserves to porewater, structural material to detritus, and 
 *  consumption of oxygen when photosynthates are released during mortality.
 *
 *  Model description: See zooxanthallae equations in:
 * 
 *  Baird, M. E., M. Mongin, F. Rizwi, L. K. Bay, N. E. Cantin, M. Soja-Wozniak and J. Skerratt (2018) 
 *  A mechanistic model of coral bleaching due to temperature-mediated light-driven reactive oxygen 
 *  build-up in zooxanthellae. Ecol. Model 386: 20-37.
 *  
 *  Copyright:
 *  Copyright (c) 2018. Commonwealth Scientific and Industrial
 *  Research Organisation (CSIRO). ABN 41 687 119 230. All rights
 *  reserved. See the license file for disclaimer and full
 *  use/redistribution conditions.
 *  
 *  $Id: phytoplankton_spectral_mortality_sed.c 5934 2018-09-12 03:34:13Z bai155 $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "ecology_internal.h"
#include "stringtable.h"
#include "utils.h"
#include "ecofunct.h"
#include "constants.h"
#include "eprocess.h"
#include "cell.h"
#include "column.h"
#include "einterface.h"
#include "phytoplankton_spectral_mortality_sed.h"

typedef struct {
  int do_mb;                  /* flag */

  /* flag: 1 for large phytoplankton, 0 for small phytoplankton */

  int large;

  /* parameters */

  double mL_t0;
  double KO_aer;

  /* tracers */

  int NH4_i;
  int NH4_pr_i;
  int DIP_i;
  int DIC_i;
  int Phy_N_i;
  int Phy_NR_i;
  int Phy_I_i;
  int DetPL_N_i;
  int TN_i;
  int TP_i;
  int TC_i;
  int Oxygen_i;
  int BOD_i;
  int COD_i;

  int Phy_PR_i;
  int Phy_Chl_i;

    /* common cell variables */

  int Tfactor_i;
  int mL_i;

} workspace;

void phytoplankton_spectral_mortality_sed_init(eprocess* p)
{
  ecology* e = p->ecology;
  stringtable* tracers = e->tracers;
  workspace* ws = malloc(sizeof(workspace));
  char* prm = p->prms->se[0]->s;
  
  p->workspace = ws;

  if (toupper(prm[0]) == 'L')
    ws->large = 1;
  else if (toupper(prm[0]) == 'S')
    ws->large = 0;
  else
    e_quit("error: ecology: \"%s\": \"%s(%s)\": unexpected parameter: expected \"small\" or \"large\"\n", e->processfname, p->name, prm);

  /*  parameters */
  ws->mL_t0 = try_parameter_value(e, (ws->large) ? "PhyL_mL_sed" : "PhyS_mL_sed");
  if (isnan(ws->mL_t0))
    ws->mL_t0 = get_parameter_value(e, (ws->large) ? "PhyL_mL" : "PhyS_mL");

  ws->KO_aer = get_parameter_value(e, "KO_aer");

  /* tracers */

  ws->Phy_N_i = e->find_index(tracers, (ws->large) ? "PhyL_N" : "PhyS_N", e);
  ws->Phy_NR_i = e->find_index(tracers, (ws->large) ? "PhyL_NR" : "PhyS_NR", e);
  ws->Phy_I_i = e->find_index(tracers, (ws->large) ? "PhyL_I" : "PhyS_I", e);
  ws->DetPL_N_i = e->find_index(tracers, "DetPL_N", e);
  ws->NH4_i = e->find_index(tracers, "NH4", e);
  ws->DIP_i = e->find_index(tracers, "DIP", e);
  ws->DIC_i = e->find_index(tracers, "DIC", e);
  ws->Oxygen_i = e->find_index(tracers, "Oxygen", e);
  ws->TN_i = e->find_index(tracers, "TN", e);
  ws->TP_i = e->find_index(tracers, "TP", e);
  ws->TC_i = e->find_index(tracers, "TC", e);
  ws->BOD_i = e->find_index(tracers, "BOD", e);
  ws->COD_i = e->try_index(tracers, "COD", e);

  ws->Phy_PR_i = e->find_index(tracers, (ws->large) ? "PhyL_PR" : "PhyS_PR", e);
  ws->Phy_Chl_i = e->find_index(tracers, (ws->large) ? "PhyL_Chl" : "PhyS_Chl", e);

  /* common cell variables */

  ws->Tfactor_i = try_index(e->cv_cell, "Tfactor", e);
  ws->mL_i = find_index_or_add(e->cv_cell, (ws->large) ? "PhyL_mL" : "PhyS_mL", e);


  /*non essentail diagnostic tracer*/

  ws->NH4_pr_i = e->try_index(tracers, "NH4_pr", e);

}

void phytoplankton_spectral_mortality_sed_postinit(eprocess* p)
{
  ecology* e = p->ecology;
  workspace* ws = (workspace*) p->workspace;

  ws->do_mb = (try_index(e->cv_model, "massbalance_sed", e) >= 0) ? 1 : 0;
}

void phytoplankton_spectral_mortality_sed_destroy(eprocess* p)
{
  free(p->workspace);
}

void phytoplankton_spectral_mortality_sed_precalc(eprocess* p, void* pp)
{
  workspace* ws = p->workspace;
  cell* c = (cell*) pp;
  double* cv = c->cv;

  double Tfactor = (ws->Tfactor_i >= 0) ? cv[ws->Tfactor_i] : 1.0;

  cv[ws->mL_i] = ws->mL_t0 * Tfactor;

  if (ws->do_mb) {
    double* y = c->y;
    double Phy_N = y[ws->Phy_N_i];
    double Phy_NR = y[ws->Phy_NR_i];
    double Phy_PR = y[ws->Phy_PR_i];
    double Phy_I = y[ws->Phy_I_i];

    y[ws->TN_i] += Phy_N + Phy_NR;
    y[ws->TP_i] += Phy_N * red_W_P + Phy_PR;
    y[ws->TC_i] += Phy_N * red_W_C + Phy_I*106.0/1060.0*12.01;
    y[ws->BOD_i] += (Phy_N * red_W_C + Phy_I*106.0/1060.0*12.01)*C_O_W;
  }
}

void phytoplankton_spectral_mortality_sed_calc(eprocess* p, void* pp)
{
  workspace* ws = p->workspace;
  intargs* ia = (intargs*) pp;
  cell* c = (cell*) ia->media;
  double* cv = ((cell*) ia->media)->cv;
  double* y = ia->y;
  double* y1 = ia->y1;
  
  double porosity = c->porosity;
  double Phy_N = y[ws->Phy_N_i];
  double Phy_NR = y[ws->Phy_NR_i];
  double Phy_PR = y[ws->Phy_PR_i];
  double Phy_I = y[ws->Phy_I_i];
  double Phy_Chl = y[ws->Phy_Chl_i];
  double Oxygen = e_max(y[ws->Oxygen_i]);
  double mortality1 = Phy_N * cv[ws->mL_i];
  double mortality2 = Phy_NR * cv[ws->mL_i];
  double mortality3 = Phy_I * cv[ws->mL_i];
  double mortality4 = Phy_PR * cv[ws->mL_i];

 /* Structural material (Phy_N) released at Redfield to DetPL, but all 
    reserves must go to individual pools (NR, PR), or lost (I, Chl)  */

  y1[ws->Phy_N_i] -= mortality1;
  y1[ws->Phy_NR_i] -= mortality2;
  y1[ws->Phy_I_i] -= mortality3;
  y1[ws->Phy_PR_i] -= mortality4;
  y1[ws->Phy_Chl_i] -= Phy_Chl * cv[ws->mL_i];
  
  y1[ws->DetPL_N_i] += mortality1 ;
  y1[ws->NH4_i] += mortality2 / porosity ;

  if (ws->  NH4_pr_i> -1)
    y1[ws->NH4_pr_i] += mortality2 * SEC_PER_DAY * c->dz_sed * porosity;

  y1[ws->DIP_i] += mortality4 / porosity;
  y1[ws->DIC_i] += mortality3 * 106.0/1060.0*12.01 / porosity;

  double Oxygen2 = Oxygen * Oxygen; 
  double sigmoid = Oxygen2 / (ws->KO_aer * ws->KO_aer + Oxygen2 );

  y1[ws->Oxygen_i] -= mortality3 * 106.0/1060.0*12.01 * C_O_W * sigmoid / porosity;

  if (ws->COD_i > -1){
    y1[ws->COD_i] += mortality3 * 106.0/1060.0*12.01 * C_O_W * (1.0-sigmoid)/ porosity;
  }
}

void phytoplankton_spectral_mortality_sed_postcalc(eprocess* p, void* pp)
{
  cell* c = ((cell*) pp);
  workspace* ws = p->workspace;
  double* y = c->y;
  
  double Phy_N = y[ws->Phy_N_i];
  double Phy_NR = y[ws->Phy_NR_i];
  double Phy_PR = y[ws->Phy_PR_i]; 
  double Phy_I = y[ws->Phy_I_i];
  
  y[ws->TN_i] += Phy_N + Phy_NR;
  y[ws->TP_i] += Phy_N * red_W_P + Phy_PR;
  y[ws->TC_i] += Phy_N * red_W_C + Phy_I*106.0/1060.0*12.01;
  y[ws->BOD_i] += (Phy_N * red_W_C + Phy_I*106.0/1060.0*12.01)*C_O_W;
}
