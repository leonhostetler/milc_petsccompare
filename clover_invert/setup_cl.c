/******** setup_cl.c *********/
/* MIMD version 7 */
#define IF_OK if(status==0)

/* Modifications ...

   8/01/98 Provision for serial write/read of scratch files C.D.
   8/30/96 Added reload_parallel for gauge fields C.D.
   8/15/96 Added prompts and param member for variable scratch file name C.D. 
   8/15/96 Added unitarity checking C.D.
   8/10/96 Revised propagator IO prompts and param file names C.D. */

//  $Log: setup_cl.c,v $
//  Revision 1.7  2006/11/07 05:28:10  detar
//  Train error files
//
//  Revision 1.6  2006/11/07 02:30:53  detar
//  Fix some omissions to complete the previous update.
//
//  Revision 1.5  2006/11/04 23:50:03  detar
//  Add file pointer to io_helpers utilities.
//
//  Revision 1.4  2006/02/20 23:47:19  detar
//  Update to version 7 and add hopping parameter inversion option
//


#include "cl_inv_includes.h"
#include <string.h>
int initial_set();

/* Each node has a params structure for passing simulation parameters */
#include "params.h"
params par_buf;

int  setup_cl()   {
  int prompt;

  /* print banner, get volume */
  prompt=initial_set();
  /* Initialize the layout functions, which decide where sites live */
  setup_layout();
  /* allocate space for lattice, set up coordinate fields */
  make_lattice();
  /* set up nearest neighbor gathers */
  make_nn_gathers();
  
  return(prompt);
}


/* SETUP ROUTINES */
int initial_set(){
  int prompt,status;
  /* On node zero, read lattice size and send to others */
  if(mynode()==0){
    /* print banner */
    printf("SU3 Wilson valence fermions\n");
    printf("MIMD version 7 $Name:  $\n");
    printf("Machine = %s, with %d nodes\n",machine_type(),numnodes());
    time_stamp("start");
    
    status = get_prompt(stdin,  &prompt );
    
    IF_OK status += get_i(stdin,prompt,"nx", &par_buf.nx );
    IF_OK status += get_i(stdin,prompt,"ny", &par_buf.ny );
    IF_OK status += get_i(stdin,prompt,"nz", &par_buf.nz );
    IF_OK status += get_i(stdin,prompt,"nt", &par_buf.nt );
    
    if(status>0) par_buf.stopflag=1; else par_buf.stopflag=0;
  } /* end if(mynode()==0) */

  /* Node 0 broadcasts parameter buffer to all other nodes */
  broadcast_bytes((char *)&par_buf,sizeof(par_buf));

  if( par_buf.stopflag != 0 )
    normal_exit(0);

  nx=par_buf.nx;
  ny=par_buf.ny;
  nz=par_buf.nz;
  nt=par_buf.nt;
  
  this_node = mynode();
  number_of_nodes = numnodes();
  volume=nx*ny*nz*nt;
  return(prompt);
}

/* read in parameters and coupling constants	*/
int readin(int prompt) {
  /* read in parameters for su3 monte carlo	*/
  /* argument "prompt" is 1 if prompts are to be given for input	*/
  
  int status,status2;
  char savebuf[128];
  char save_w[128];
  int i;
  char descrp[30];

  /* On node zero, read parameters and send to all other nodes */
  if(this_node==0){
    
    printf("\n\n");
    status=0;
    
    /* Number of kappas */
    IF_OK status += get_i(stdin,prompt,"number_of_kappas", &par_buf.num_kap );
    if( par_buf.num_kap>MAX_KAP ){
      printf("num_kap = %d must be <= %d!\n", par_buf.num_kap, MAX_KAP);
      status++;
    }
    
    /* To be safe initialize the following to zero */
    for(i=0;i<MAX_KAP;i++){
      kap[i] = 0.0;
      resid[i] = 0.0;
    }
    
    for(i=0;i<par_buf.num_kap;i++){
      IF_OK status += get_f(stdin, prompt,"kappa", &par_buf.kap[i] );
    }
    
    /* Clover coefficient, u0 */
    IF_OK status += get_f(stdin, prompt,"clov_c", &par_buf.clov_c );
    IF_OK status += get_f(stdin, prompt,"u0", &par_buf.u0 );
    
    /* maximum no. of conjugate gradient iterations */
    IF_OK status += get_i(stdin,prompt,"max_cg_iterations", &par_buf.niter );
    
    /* maximum no. of conjugate gradient restarts */
    IF_OK status += get_i(stdin,prompt,"max_cg_restarts", &par_buf.nrestart );
    
    /* error for propagator conjugate gradient */
    for(i=0;i<par_buf.num_kap;i++){
      IF_OK status += get_f(stdin, prompt,"error_for_propagator", &par_buf.resid[i] );
    }
    
    /* Get source type */
    IF_OK status += ask_quark_source(prompt,&wallflag,descrp);

    /* width: psi=exp(-(r/r0)^2) */
    IF_OK if (prompt!=0) 
      printf("enter width(s) r0 as in: source=exp(-(r/r0)^2)\n");
    for(i=0;i<par_buf.num_kap;i++){
      IF_OK status += get_f(stdin, prompt,"r0", &par_buf.wqs[i].r0 );
	/* (Same source type for each spectator) */
	IF_OK par_buf.wqs[i].type = wallflag;
	IF_OK strcpy(par_buf.wqs[i].descrp,descrp);
	/* (Hardwired source location for each spectator) */
	IF_OK {
	  par_buf.wqs[i].x0 = source_loc[0];
	  par_buf.wqs[i].y0 = source_loc[1];
	  par_buf.wqs[i].z0 = source_loc[2];
	  par_buf.wqs[i].t0 = source_loc[3];
	}
    }
    
    /* find out what kind of starting lattice to use */
    IF_OK status += ask_starting_lattice(stdin,  prompt, &par_buf.startflag,
	par_buf.startfile );

    IF_OK if (prompt!=0) 
      printf("enter 'no_gauge_fix', or 'coulomb_gauge_fix'\n");
    IF_OK scanf("%s",savebuf);
    IF_OK printf("%s\n",savebuf);
    IF_OK {
      if(strcmp("coulomb_gauge_fix",savebuf) == 0 ){
	par_buf.fixflag = COULOMB_GAUGE_FIX;
      }
      else if(strcmp("no_gauge_fix",savebuf) == 0 ) {
	par_buf.fixflag = NO_GAUGE_FIX;
      }
      else{
	printf("error in input: fixing_command is invalid\n"); status++;
      }
    }
    
    /* find out what to do with lattice at end */
    IF_OK status += ask_ending_lattice(stdin,  prompt, &(par_buf.saveflag),
			     par_buf.savefile );
    IF_OK status += ask_ildg_LFN(stdin,  prompt, par_buf.saveflag,
				  par_buf.stringLFN );
    
    /* find out starting propagator */
    IF_OK for(i=0;i<par_buf.num_kap;i++)
      status += ask_starting_wprop( prompt,&par_buf.startflag_w[i],
			par_buf.startfile_w[i]);
    
    /* what to do with computed propagator */
    IF_OK for(i=0;i<par_buf.num_kap;i++)
      status += ask_ending_wprop( prompt,&par_buf.saveflag_w[i],
		      par_buf.savefile_w[i]);
    
    IF_OK if(prompt!=0) 
      printf("propagator scratch file:\n enter 'serial_scratch_wprop', 'parallel_scratch_wprop' or 'multidump_scratch_wprop'\n");
    IF_OK status2=scanf("%s",save_w);
    IF_OK printf("%s ",save_w);
    IF_OK 
      {
	if(strcmp("serial_scratch_wprop",save_w) == 0 )
	  par_buf.scratchflag = SAVE_SERIAL;
	else if(strcmp("parallel_scratch_wprop",save_w) == 0 )
	  par_buf.scratchflag = SAVE_CHECKPOINT;
	else if(strcmp("multidump_scratch_wprop",save_w) == 0 )
	  par_buf.scratchflag = SAVE_MULTIDUMP;
	else
	  {
	    printf("error in input: %s is not a scratch file command\n",save_w);
	    status++;
	  }
	IF_OK
	  {
	    /*read name of file and load it */
	    if(prompt!=0)printf("enter name of scratch file stem for props\n");
	    status2=scanf("%s",par_buf.scratchstem_w);
	    if(status2 !=1) {
	      printf("error in input: scratch file stem name\n"); status++;
	    }
	    printf("%s\n",par_buf.scratchstem_w);
	  }
      }
    
    if( status > 0)par_buf.stopflag=1; else par_buf.stopflag=0;
  } /* end if(this_node==0) */
  
  broadcast_bytes((char *)&par_buf,sizeof(par_buf));
  if( par_buf.stopflag != 0 )
    normal_exit(0);

  startflag = par_buf.startflag;
  fixflag = par_buf.fixflag;
  saveflag = par_buf.saveflag;
  for(i=0;i<par_buf.num_kap;i++){
    startflag_w[i] = par_buf.startflag_w[i];
    saveflag_w[i] = par_buf.saveflag_w[i];
  }
  niter = par_buf.niter;
  nrestart = par_buf.nrestart;
  num_kap = par_buf.num_kap;
  clov_c = par_buf.clov_c;
  u0 = par_buf.u0;
  for(i=0;i<par_buf.num_kap;i++){
    kap[i] = par_buf.kap[i];
    resid[i] = par_buf.resid[i];
    wqs[i] = par_buf.wqs[i];
  }
  strcpy(startfile,par_buf.startfile);
  strcpy(savefile,par_buf.savefile);
  for(i=0;i<par_buf.num_kap;i++){
    strcpy(startfile_w[i],par_buf.startfile_w[i]);
    strcpy(savefile_w[i],par_buf.savefile_w[i]);
  }
  strcpy(scratchstem_w,par_buf.scratchstem_w);
  scratchflag = par_buf.scratchflag;
  
  /* Do whatever is needed to get lattice */
  startlat_p = reload_lattice( startflag, startfile );

  return(0);
}

