/************************* control.c *******************************/
/* MIMD version 7 */
/* Main procedure for SU3 with dynamical staggered fermions        */
/* general quark action, general gauge action */

/* This file is for lattice generation with the RHMC algorithm */

#define CONTROL
#include "ks_imp_includes.h"	/* definitions files and prototypes */
#include "lattice_qdp.h"

#ifdef HAVE_QUDA
#include <quda_milc_interface.h>
#include "../include/generic_quda.h"
#endif

#ifdef HAVE_QPHIX
#include "../include/generic_qphix.h"
#endif

#ifdef MILC_GLOBAL_DEBUG
#include "debug.h"
#endif /* MILC_GLOBAL_DEBUG */

/* For information */
#define NULL_FP -1

EXTERN gauge_header start_lat_hdr;	/* Input gauge field header */

int
main( int argc, char **argv )
{
  int i,meascount,traj_done, naik_index;
  int prompt;
  int s_iters, avs_iters, avbcorr_iters;
  double starttime, endtime;
#ifdef PRTIME
  double dtime;
#endif
  
  initialize_machine(&argc,&argv);
  
  /* Remap standard I/O */
  if(remap_stdio_from_args(argc, argv) == 1)terminate(1);
  
  g_sync();
  
  starttime = dclock();
  
  /* set up */
  STARTTIME;
  prompt = setup();
  ENDTIME("setup");
  
  /* loop over input sets */
  while( readin(prompt) == 0) {
    
    /* perform warmup trajectories */
#ifdef MILC_GLOBAL_DEBUG
    global_current_time_step = 0;
#endif /* MILC_GLOBAL_DEBUG */
    
    for( traj_done=0; traj_done < warms; traj_done++ ){
      update();
    }
    node0_printf("WARMUPS COMPLETED\n"); fflush(stdout);
    
    /* perform measuring trajectories, reunitarizing and measuring 	*/
    meascount=0;		/* number of measurements 		*/
    avs_iters = avbcorr_iters = 0;
    
    for( traj_done=0; traj_done < trajecs; traj_done++ ){ 
#ifdef MILC_GLOBAL_DEBUG
#ifdef HISQ_REUNITARIZATION_DEBUG
      {
	int isite, idir;
	site *s;
	FORALLSITES(isite,s) {
	  for( idir=XUP;idir<=TUP;idir++ ) {
	    lattice[isite].on_step_Y[idir] = 0;
	    lattice[isite].on_step_W[idir] = 0;
	    lattice[isite].on_step_V[idir] = 0;
	  }
	}
      }
#endif /* HISQ_REUNITARIZATION_DEBUG */
#endif /* MILC_GLOBAL_DEBUG */
      /* do the trajectories */
      STARTTIME;
      s_iters=update();
      ENDTIME("do one trajectory");
      
      /* measure every "propinterval" trajectories */
      if( (traj_done%propinterval)==(propinterval-1) ){
	
	/* call gauge_variable fermion_variable measuring routines */
	/* results are printed in output file */
	STARTTIME;
	g_measure_ks( );
	ENDTIME("do gauge measurement");
#ifdef MILC_GLOBAL_DEBUG
#if FERM_ACTION == HISQ
        g_measure_plaq( );
#endif
#ifdef MEASURE_AND_TUNE_HISQ
        g_measure_tune( );
#endif /* MEASURE_AND_TUNE_HISQ */
#endif /* MILC_GLOBAL_DEBUG */
	
	
	/**************************************************************/
	/* Compute chiral condensate and related quantities           */
	
	/* Make fermion links if not already done */
	
	STARTTIME;
	invalidate_fermion_links(fn_links);
	restore_fermion_links_from_site(fn_links, param.prec_pbp);
	for(i = 0; i < param.num_pbp_masses; i++){
#if FERM_ACTION == HISQ
	  naik_index = param.ksp_pbp[i].naik_term_epsilon_index;
#else
	  naik_index = 0;
#endif
 	  f_meas_imp_field( param.npbp_reps, &param.qic_pbp[i], 
 			    param.ksp_pbp[i].mass, naik_index, fn_links);
	  
#ifdef D_CHEM_POT
	  Deriv_O6_field( param.npbp_reps, &param.qic_pbp[i],
			  param.ksp_pbp[i].mass, naik_index, fn_links);
#endif
	}
	ENDTIME("do pbp measurements");
	avs_iters += s_iters;
	++meascount;
	fflush(stdout);
      }
    }	/* end loop over trajectories */
    
    node0_printf("RUNNING COMPLETED\n"); fflush(stdout);
    if(meascount>0)  {
      node0_printf("average cg iters for step= %e\n",
		   (double)avs_iters/meascount);
    }
    
    endtime = dclock();
    if(this_node==0){
      printf("Time = %e seconds\n",(double)(endtime-starttime));
      printf("total_iters = %d\n",total_iters);
#ifdef HISQ_SVD_COUNTER
      printf("hisq_svd_counter = %d\n",hisq_svd_counter);
#endif
      
#ifdef HISQ_FORCE_FILTER_COUNTER
      printf("hisq_force_filter_counter = %d\n",hisq_force_filter_counter);
#endif
    }
    fflush(stdout);
    starttime = endtime;
    
    /* save lattice if requested */
    if( saveflag != FORGET ){
      rephase( OFF );
      save_lattice( saveflag, savefile, stringLFN );
      rephase( ON );
    }
    
    /* Destroy fermion links (created in readin() */
    
#if FERM_ACTION == HISQ
    destroy_fermion_links_hisq(fn_links);
#endif
    fn_links = NULL;

  }
  free_lattice();
  
#ifdef HAVE_QUDA
  finalize_quda();
#endif
  
#ifdef HAVE_QPHIX
  finalize_qphix();
#endif
  
  normal_exit(0);
  return 0;
}

