/******* d_congrad5_fn.c - conjugate gradient for SU3/fermions ****/
/* MIMD version 7 */
/* Kogut-Susskind fermions -- this version for "fat plus Naik" quark
   actions.  

   Previously called d_congrad5_fn_fewsums.c and d_congrad5_fn_tmp.c
   Previous functionality now appears in d_congrad5_fn_milc.c.
*/

#include "generic_ks_includes.h"
#include "../include/fermion_links.h"
#include "../include/openmp_defs.h"
#include <string.h>

static int
zero_source( su3_vector *src, su3_vector *dest,
		  quark_invert_control *qic ){
  int i;
  double source_norm = 0.;
  int parity = qic->parity;

  FORSOMEFIELDPARITY_OMP(i,parity,reduction(+:source_norm)){
    source_norm += (double)magsq_su3vec( src + i );
  } END_LOOP_OMP;
  g_doublesum( &source_norm );

  if(source_norm > 0)
    return 0;

  /* Source is zero so answer is zero */
  FORSOMEFIELDPARITY_OMP(i,parity,){
    memset(dest + i, 0, sizeof(su3_vector));
  } END_LOOP_OMP;

  qic->size_r = 0;
  qic->size_relr = 1.;
  qic->final_iters   = 0;
  qic->final_restart = 0;
  qic->converged     = 1;
  qic->final_rsq = 0.;
  qic->final_relrsq = 0.;

  return 1;
}


static int
zero_block_source( int nsrc, su3_vector **src,
			su3_vector **dest,
			quark_invert_control *qic ){
  int i;
  double source_norm = 0.;
  int parity = qic->parity;
  
  for(int is = 0; is < nsrc; is++){
    FORSOMEFIELDPARITY_OMP(i,parity,reduction(+:source_norm)){
      source_norm += (double)magsq_su3vec( src[is] + i );
    } END_LOOP_OMP;
  }
  g_doublesum( &source_norm );

  if(source_norm > 0)
    return 0;

  /* Source is zero so answer is zero */
  for(int is = 0; is < nsrc; is++){
    FORSOMEFIELDPARITY_OMP(i,parity,){
      memset(dest[is] + i, 0, sizeof(su3_vector));
    } END_LOOP_OMP;
  }

  qic->size_r = 0;
  qic->size_relr = 1.;
  qic->final_iters   = 0;
  qic->final_restart = 0;
  qic->converged     = 1;
  qic->final_rsq = 0.;
  qic->final_relrsq = 0.;

  return 1;
}


/* API for field arguments */

int ks_congrad_field( su3_vector *src, su3_vector *dest, 
		      quark_invert_control *qic, Real mass,
		      imp_ferm_links_t *fn)
{
  int iters = 0;
  int parity = qic->parity;

  if(parity == EVEN || parity == EVENANDODD){
    qic->parity = EVEN;
    if( ! zero_source(src, dest, qic) )
      iters += ks_congrad_parity(src, dest, qic, mass, fn);
    report_status(qic);
  }
  if(parity == ODD || parity == EVENANDODD){
    qic->parity = ODD;
    if( ! zero_source(src, dest, qic) )
      iters += ks_congrad_parity(src, dest, qic, mass, fn);
    report_status(qic);
  }

  qic->parity = parity;
  return iters;
}

/* API for block field arguments */

int ks_congrad_block_field( int nsrc, su3_vector **src, su3_vector **dest, 
			    quark_invert_control *qic, Real mass,
			    imp_ferm_links_t *fn)
{
  int iters = 0;
  int parity = qic->parity;

  if(parity == EVEN || parity == EVENANDODD){
    qic->parity = EVEN;
    if( ! zero_block_source(nsrc, src, dest, qic) )
      iters += ks_congrad_block_parity(nsrc, src, dest, qic, mass, fn);
    report_status(qic);
  }
  if(parity == ODD || parity == EVENANDODD){
    qic->parity = ODD;
    if( ! zero_block_source(nsrc, src, dest, qic) )
      iters += ks_congrad_block_parity(nsrc, src, dest, qic, mass, fn);
    report_status(qic);
  }

  qic->parity = parity;
  return iters;
}

/* API for field arguments.  This one never uses the GPU. */

int ks_congrad_field_cpu( su3_vector *src, su3_vector *dest, 
			  quark_invert_control *qic, Real mass,
			  imp_ferm_links_t *fn)
{
  int iters = 0;
  int parity = qic->parity;

  if(parity == EVEN || parity == EVENANDODD){
    qic->parity = EVEN;
    iters += ks_congrad_parity_cpu(src, dest, qic, mass, fn);
    report_status(qic);
  }
  if(parity == ODD || parity == EVENANDODD){
    qic->parity = ODD;
    iters += ks_congrad_parity_cpu(src, dest, qic, mass, fn);
    report_status(qic);
  }

  qic->parity = parity;
  return iters;
}

/* API for site arguments */

int ks_congrad_site( field_offset src, field_offset dest, 
		     quark_invert_control *qic, Real mass,
		     imp_ferm_links_t *fn)
{
  int iters = 0;
  su3_vector *t_src, *t_dest;
  int parity = qic->parity;

  /* Map src and dest from site to field of correct precision */
  
  t_src  = create_v_field_from_site_member(src);
  t_dest = create_v_field_from_site_member(dest);

  if(parity == EVEN || parity == EVENANDODD){
    qic->parity = EVEN;
    iters += ks_congrad_parity(t_src, t_dest, qic, mass, fn );
    report_status(qic);
  }
  if(parity == ODD || parity == EVENANDODD){
    qic->parity = ODD;
    iters += ks_congrad_parity(t_src, t_dest, qic, mass, fn );
    report_status(qic);
  }

  /* Map solution to site structure */

  copy_site_member_from_v_field(dest, t_dest);

  qic->parity = parity;

  destroy_v_field(t_src); destroy_v_field(t_dest);

  return iters;
}

/* Traditional MILC API for site arguments and no relative residual test */

int ks_congrad( field_offset src, field_offset dest, Real mass,
		int niter, int nrestart, Real rsqmin, int prec,
		int parity, Real *final_rsq,
		imp_ferm_links_t *fn){
  int iters;
  quark_invert_control qic;

  /* Pack structure */
  qic.prec      = prec;  /* Currently ignored */
  qic.min       = 0;
  qic.max       = niter;
  qic.nrestart  = nrestart;
  qic.parity    = parity;
  qic.nsrc      = 1;
  qic.resid     = sqrt(rsqmin);
  qic.relresid  = 0;     /* Suppresses this test */

  /* Solve the system */
  iters = ks_congrad_site( src, dest, &qic, mass, fn );

  /* Unpack the results */
  *final_rsq    = qic.final_rsq;
  return iters;
}
