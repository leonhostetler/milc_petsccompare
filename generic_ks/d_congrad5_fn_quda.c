/******* d_congrad5_fn_gpu.c - conjugate gradient for SU3/fermions ****/
/* MIMD version 7 */

/* GPU version of d_congrad5_fn_milc.c.  Can be compiled together with it. */
// Note that the restart criterion used by QUDA is different from 
// the restart criterion used by the MILC code

#include "generic_ks_includes.h"
#include "../include/prefetch.h"
#define FETCH_UP 1

#define LOOPEND
#include "../include/loopend.h"
#include <string.h>
#include "../include/generic.h"

#include <quda_milc_interface.h>
#include "../include/generic_quda.h"

#ifdef CGTIME
static const char *prec_label[2] = {"F", "D"};
#endif

/* Backward compatibility*/
#ifdef SINGLE_FOR_DOUBLE
#define HALF_MIXED
#endif

/********************************************************************/
/* Solution of the normal equations for a single site parity        */
/********************************************************************/

// Note that the restart criterion used by QUDA is different from 
// the restart criterion used by the MILC code
int ks_congrad_parity_gpu(su3_vector *t_src, su3_vector *t_dest, 
			  quark_invert_control *qic, Real mass,
			  imp_ferm_links_t *fn)
{

  char myname[] = "ks_congrad_parity_gpu";
  QudaInvertArgs_t inv_args;
  int i;
  double dtimec = -dclock();
#ifdef CGTIME
  double nflop = 1187;
#endif

//  if(qic->relresid != 0.){
//    printf("%s: GPU code does not yet support a Fermilab-type relative residual\n",myname);
//    terminate(1);
//  }
 
  /* Initialize qic */
  qic->size_r = 0;
  qic->size_relr = 0;
  qic->final_iters   = 0;
  qic->final_restart = 0;
  qic->converged     = 1;
  qic->final_rsq = 0.;
  qic->final_relrsq = 0.;

  /* Compute source norm */
  double source_norm = 0.0;
  FORSOMEFIELDPARITY(i,qic->parity){
    source_norm += (double)magsq_su3vec( &t_src[i] );
  } END_LOOP;
  g_doublesum( &source_norm );
#ifdef CG_DEBUG
  node0_printf("congrad: source_norm = %e\n", (double)source_norm);
#endif

  /* Provide for trivial solution */
  if(source_norm == 0.0){
    /* Zero the solution and return zero iterations */
    FORSOMEFIELDPARITY(i,qic->parity){
      memset(t_dest + i, 0, sizeof(su3_vector));
    } END_LOOP;

    dtimec += dclock();
#ifdef CGTIME
    if(this_node==0){
      printf("CONGRAD5: time = %e (fn_QUDA %s) masses = 1 iters = %d mflops = %e\n",
	     dtimec, prec_label[qic->prec-1], qic->final_iters, 
	     (double)(nflop*volume*qic->final_iters/(1.0e6*dtimec*numnodes())) );
      fflush(stdout);}
#endif
    
    return 0;
  }

  /* Initialize QUDA parameters */

  initialize_quda();
 
  if(qic->parity == EVEN){
	  inv_args.evenodd = QUDA_EVEN_PARITY;
  }else if(qic->parity == ODD){
	  inv_args.evenodd = QUDA_ODD_PARITY;
  }else{
    printf("%s: Unrecognised parity\n",myname);
    terminate(2);
  }

  inv_args.max_iter = qic->max*qic->nrestart;
#if defined(MAX_MIXED)
  inv_args.mixed_precision = 2;
#elif defined(HALF_MIXED)
  inv_args.mixed_precision = 1;
#else
  inv_args.mixed_precision = 0;
#endif

  su3_matrix* fatlink = get_fatlinks(fn);
  su3_matrix* longlink = get_lnglinks(fn);
  const int quda_precision = qic->prec;

  double residual, relative_residual;
  int num_iters = 0;

  // for newer versions of QUDA we need to invalidate the gauge field if the links are new
  if ( fn != get_fn_last() || fresh_fn_links(fn) ){
    cancel_quda_notification(fn);
    set_fn_last(fn);
    num_iters = -1;  /* This hack signals QUDA */
    node0_printf("%s: fn, notify: Signal QUDA to refresh links\n", myname);
  }

  inv_args.naik_epsilon = fn->eps_naik;

#if (FERM_ACTION==HISQ)
  inv_args.tadpole = 1.0;
#else
  inv_args.tadpole = u0;
#endif

  // Setup for deflation (and eigensolve) on GPU
  int parity = qic->parity;
  int blockSize = param.eigen_param.blockSize;
  static Real previous_mass = -1.0;
  static bool first_solve=true;

  QudaEigParam qep = newQudaEigParam();
  qep.block_size = blockSize;
  qep.eig_type = ( blockSize > 1 ) ? QUDA_EIG_BLK_TR_LANCZOS : QUDA_EIG_TR_LANCZOS;  /* or QUDA_EIG_IR_ARNOLDI, QUDA_EIG_BLK_IR_ARNOLDI */
  qep.spectrum = QUDA_SPECTRUM_SR_EIG; /* Smallest Real. Other options: LM, SM, LR, SR, LI, SI */
  qep.n_conv = (param.eigen_param.Nvecs_in > param.eigen_param.Nvecs) ? param.eigen_param.Nvecs_in : param.eigen_param.Nvecs;
  qep.n_ev_deflate = param.eigen_param.Nvecs;
  qep.n_ev = qep.n_conv;
  qep.n_kr = (param.eigen_param.Nkr < qep.n_ev ) ? 2*qep.n_ev : param.eigen_param.Nkr;
  qep.tol = param.eigen_param.tol;
  qep.qr_tol = qep.tol;
  qep.max_restarts = param.eigen_param.MaxIter;
  qep.require_convergence = QUDA_BOOLEAN_TRUE;
  qep.check_interval = 10;
  qep.use_norm_op = ( parity == EVENANDODD ) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.use_pc = ( parity != EVENANDODD) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.use_dagger = QUDA_BOOLEAN_FALSE;
  qep.compute_gamma5 = QUDA_BOOLEAN_FALSE;
  qep.compute_svd = QUDA_BOOLEAN_FALSE;
  qep.use_eigen_qr = QUDA_BOOLEAN_TRUE;
  qep.use_poly_acc = QUDA_BOOLEAN_TRUE;
  qep.poly_deg = param.eigen_param.poly.norder;
  qep.a_min = param.eigen_param.poly.minE;
  qep.a_max = param.eigen_param.poly.maxE;
  qep.arpack_check = QUDA_BOOLEAN_FALSE;
  strcpy( qep.vec_infile, param.ks_eigen_startfile );
  strcpy( qep.vec_outfile, param.ks_eigen_savefile );
  qep.io_parity_inflate = QUDA_BOOLEAN_FALSE;
  qep.compute_evals_batch_size = 16;
  qep.preserve_deflation = QUDA_BOOLEAN_TRUE;
  qep.preserve_evals = ( first_solve || fabs(mass - previous_mass) < 1e-6 ) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.batched_rotate = param.eigen_param.batchedRotate;
  qep.save_prec = QUDA_SINGLE_PRECISION; // add to input parameters?
  qep.partfile = param.eigen_param.partfile ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;

  inv_args.eig_param = qep;
  if(param.eigen_param.eigPrec == 2)inv_args.prec_eigensolver = QUDA_DOUBLE_PRECISION;
  else if(param.eigen_param.eigPrec == 1)inv_args.prec_eigensolver = QUDA_SINGLE_PRECISION;
  else inv_args.prec_eigensolver = QUDA_HALF_PRECISION;

  inv_args.tol_restart = param.eigen_param.tol_restart;

  previous_mass = mass;
  first_solve = false;

  qudaInvert(MILC_PRECISION,
	     quda_precision, 
	     mass,
	     inv_args,
	     qic->resid,
	     qic->relresid,
	     fatlink, 
	     longlink,
	     t_src, 
	     t_dest,
	     &residual,
	     &relative_residual, 
	     &num_iters);

  qic->final_rsq = residual*residual;
  qic->final_relrsq = relative_residual*relative_residual;
  qic->final_iters = num_iters;

  // check for convergence 
  qic->converged = (residual < qic->resid) ? 1 : 0;

  // Cumulative residual. Not used in practice 
  qic->size_r = 0.0;
  qic->size_relr = 0.0; 

  dtimec += dclock();

#ifdef CGTIME
  if(this_node==0){
    printf("CONGRAD5: time = %e (fn_QUDA %s) masses = 1 iters = %d mflops = %e\n",
	   dtimec, prec_label[quda_precision-1], qic->final_iters, 
	   (double)(nflop*volume*qic->final_iters/(1.0e6*dtimec*numnodes())) );
    fflush(stdout);}
#endif

  return num_iters;
}

/********************************************************************/
/* Solution of the normal equations for a single site parity with   */
/* multiple right sides                                             */
/********************************************************************/

int ks_congrad_block_parity_gpu(int nsrc, su3_vector **t_src, su3_vector **t_dest, 
				quark_invert_control *qic, Real mass,
				imp_ferm_links_t *fn)
{
  if (this_node == 0) {
    printf("CONGRAD5: using QUDA's block solver\n");
  }
#if 0
  /* Until QUDA's MRHS solver is fixed we fake it */
  int num_iters = 0;
  for(int i = 0; i < nsrc; i++){
    num_iters += ks_congrad_parity_gpu(t_src[i], t_dest[i], qic, mass, fn);
  }
  return num_iters;
#else
  char myname[] = "ks_congrad_block_parity_gpu";
  QudaInvertArgs_t inv_args;
  int i;
  double dtimec = -dclock();
#ifdef CGTIME
  double nflop = 1187;  // FIXME Wrong flops
#endif

  /* Initialize qic */
  qic->size_r = 0;
  qic->size_relr = 0;
  qic->final_iters   = 0;
  qic->final_restart = 0;
  qic->converged     = 1;
  qic->final_rsq = 0.;
  qic->final_relrsq = 0.;

  /* Compute source norm */
  double source_norm = 0.0;
  FORSOMEFIELDPARITY(i,qic->parity){
    source_norm += (double)magsq_su3vec( &t_src[0][i] );
  } END_LOOP;
  g_doublesum( &source_norm );
#ifdef CG_DEBUG
  node0_printf("congrad: source_norm = %e\n", (double)source_norm);
#endif

  /* Provide for trivial solution */
  if(source_norm == 0.0){
    /* Zero the solution and return zero iterations */
    FORSOMEFIELDPARITY(i,qic->parity){
      memset(t_dest + i, 0, sizeof(su3_vector));
    } END_LOOP;
    
    dtimec += dclock();
#ifdef CGTIME
    if(this_node==0){
      printf("CONGRAD5: time = %e (fn_QUDA %s) masses = 1 iters = %d mflops = %e\n",
	     dtimec, prec_label[MILC_PRECISION-1], qic->final_iters,
	     (double)(nflop*volume*qic->final_iters/(1.0e6*dtimec*numnodes())) );
      fflush(stdout);}
#endif
    
    return 0;
  }
  
  /* Initialize QUDA parameters */

  initialize_quda();

  if(qic->parity == EVEN){
    inv_args.evenodd = QUDA_EVEN_PARITY;
  }else if(qic->parity == ODD){
    inv_args.evenodd = QUDA_ODD_PARITY;
  }else{
    printf("%s: Unrecognised parity\n",myname);
    terminate(2);
  }

  inv_args.max_iter = qic->max*qic->nrestart;
#if defined(MAX_MIXED) || defined(HALF_MIXED)
  inv_args.mixed_precision = 1;
#else
  inv_args.mixed_precision = 0;
#endif

  su3_matrix* fatlink = get_fatlinks(fn);
  su3_matrix* longlink = get_lnglinks(fn);
  const int quda_precision = qic->prec;

  double residual, relative_residual;
  int num_iters = 0;

  // for newer versions of QUDA we need to invalidate the gauge field if the links are new
  if ( fn != get_fn_last() || fresh_fn_links(fn) ){
    cancel_quda_notification(fn);
    set_fn_last(fn);
    num_iters = -1;
    node0_printf("%s: fn, notify: Signal QUDA to refresh links\n", myname);
  }

  inv_args.naik_epsilon = fn->eps_naik;

#if (FERM_ACTION==HISQ)
  inv_args.tadpole = 1.0;
#else
  inv_args.tadpole = u0;
#endif

  // Setup for deflation (and eigensolve) on GPU
  int parity = qic->parity;
  int blockSize = param.eigen_param.blockSize;
  static Real previous_mass = -1.0;
  static bool first_solve=true;

  QudaEigParam qep = newQudaEigParam();
  qep.block_size = blockSize;
  qep.eig_type = ( blockSize > 1 ) ? QUDA_EIG_BLK_TR_LANCZOS : QUDA_EIG_TR_LANCZOS;  /* or QUDA_EIG_IR_ARNOLDI, QUDA_EIG_BLK_IR_ARNOLDI */
  qep.spectrum = QUDA_SPECTRUM_SR_EIG; /* Smallest Real. Other options: LM, SM, LR, SR, LI, SI */
  qep.n_conv = (param.eigen_param.Nvecs_in > param.eigen_param.Nvecs) ? param.eigen_param.Nvecs_in : param.eigen_param.Nvecs;
  qep.n_ev_deflate = param.eigen_param.Nvecs;
  qep.n_ev = qep.n_conv;
  qep.n_kr = (param.eigen_param.Nkr < qep.n_ev ) ? 2*qep.n_ev : param.eigen_param.Nkr;
  qep.tol = param.eigen_param.tol;
  qep.qr_tol = qep.tol;
  qep.max_restarts = param.eigen_param.MaxIter;
  qep.require_convergence = QUDA_BOOLEAN_TRUE;
  qep.check_interval = 10;
  qep.use_norm_op = ( parity == EVENANDODD ) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.use_pc = ( parity != EVENANDODD) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.use_dagger = QUDA_BOOLEAN_FALSE;
  qep.compute_gamma5 = QUDA_BOOLEAN_FALSE;
  qep.compute_svd = QUDA_BOOLEAN_FALSE;
  qep.use_eigen_qr = QUDA_BOOLEAN_TRUE;
  qep.use_poly_acc = QUDA_BOOLEAN_TRUE;
  qep.poly_deg = param.eigen_param.poly.norder;
  qep.a_min = param.eigen_param.poly.minE;
  qep.a_max = param.eigen_param.poly.maxE;
  qep.arpack_check = QUDA_BOOLEAN_FALSE;
  strcpy( qep.vec_infile, param.ks_eigen_startfile );
  strcpy( qep.vec_outfile, param.ks_eigen_savefile );
  qep.io_parity_inflate = QUDA_BOOLEAN_FALSE;
  qep.compute_evals_batch_size = 16;
  qep.preserve_deflation = QUDA_BOOLEAN_TRUE;
  qep.preserve_evals = ( first_solve || fabs(mass - previous_mass) < 1e-6 ) ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;
  qep.batched_rotate = param.eigen_param.batchedRotate;
  qep.save_prec = QUDA_SINGLE_PRECISION; // add to input parameters?
  qep.partfile = param.eigen_param.partfile ? QUDA_BOOLEAN_TRUE : QUDA_BOOLEAN_FALSE;

  inv_args.eig_param = qep;
  if(param.eigen_param.eigPrec == 2)inv_args.prec_eigensolver = QUDA_DOUBLE_PRECISION;
  else if(param.eigen_param.eigPrec == 1)inv_args.prec_eigensolver = QUDA_SINGLE_PRECISION;
  else inv_args.prec_eigensolver = QUDA_HALF_PRECISION;

  inv_args.tol_restart = param.eigen_param.tol_restart;

  previous_mass = mass;
  first_solve = false;

  qudaInvertMsrc(MILC_PRECISION,
     quda_precision,
     mass,
     inv_args,
     qic->resid,
     qic->relresid,
     fatlink,
     longlink,
     (void**)t_src,
     (void**)t_dest,
     &residual,
     &relative_residual,
     &num_iters,
     nsrc);


  // MILC's convention impled from d_congrad5_fn_milc.c is that final_rsq, final_relrsq, and final_iters
  // are based on the values from the last solve, which qudaInvertMsrc respects.
  qic->final_rsq = residual * residual;
  qic->final_relrsq = relative_residual * relative_residual;
  qic->final_iters = num_iters;

  // check for convergence 
  qic->converged = (residual < qic->resid) ? 1 : 0;

  // Cumulative residual. Not used in practice 
  qic->size_r = 0.0;
  qic->size_relr = 0.0;

  dtimec += dclock();

#ifdef CGTIME
  if(this_node==0){
    printf("CONGRAD5: time = %e (fn_QUDA %s) masses = 1 srcs = %d iters = %d mflops = %e\n",
           dtimec, prec_label[quda_precision-1], nsrc,qic->final_iters,
           (double)(nflop*nsrc*volume*qic->final_iters/(1.0e6*dtimec*numnodes())) );
    fflush(stdout);}
#endif

  // On the other hand, MILC expects the returned value to be the aggregate number of iterations
  // performed by each solve if it was performed sequentially. This can be approximated by
  // the number of iterations for a single solve times the number of sources.
  return num_iters * nsrc;

#endif /* if 1 */
  return num_iters;
}


