/* ----------------------------------------------------------------------------
 *
 * (c) The GHC Team, 1998-2004
 *
 * Macros for building and manipulating closures
 *
 * -------------------------------------------------------------------------- */

#ifndef RTS_STORAGE_CLOSUREMACROS_H
#define RTS_STORAGE_CLOSUREMACROS_H

/* -----------------------------------------------------------------------------
   Info tables are slammed up against the entry code, and the label
   for the info table is at the *end* of the table itself.  This
   inline function adjusts an info pointer to point to the beginning
   of the table, so we can use standard C structure indexing on it.

   Note: this works for SRT info tables as long as you don't want to
   access the SRT, since they are laid out the same with the SRT
   pointer as the first word in the table.

   NOTES ABOUT MANGLED C VS. MINI-INTERPRETER:

   A couple of definitions:

       "info pointer"    The first word of the closure.  Might point
                         to either the end or the beginning of the
			 info table, depending on whether we're using
			 the mini interpretter or not.  GET_INFO(c)
			 retrieves the info pointer of a closure.

       "info table"      The info table structure associated with a
                         closure.  This is always a pointer to the
			 beginning of the structure, so we can
			 use standard C structure indexing to pull out
			 the fields.  get_itbl(c) returns a pointer to
			 the info table for closure c.

   An address of the form xxxx_info points to the end of the info
   table or the beginning of the info table depending on whether we're
   mangling or not respectively.  So, 

         c->header.info = xxx_info 

   makes absolute sense, whether mangling or not.
 
   -------------------------------------------------------------------------- */

#define SET_INFO(c,i) ((c)->header.info = (i))
#define GET_INFO(c)   ((c)->header.info)
#define GET_ENTRY(c)  (ENTRY_CODE(GET_INFO(c)))

#define get_itbl(c)   (INFO_PTR_TO_STRUCT((c)->header.info))
#define get_ret_itbl(c) (RET_INFO_PTR_TO_STRUCT((c)->header.info))
#define get_fun_itbl(c) (FUN_INFO_PTR_TO_STRUCT((c)->header.info))
#define get_thunk_itbl(c) (THUNK_INFO_PTR_TO_STRUCT((c)->header.info))
#define get_con_itbl(c) (CON_INFO_PTR_TO_STRUCT((c)->header.info))

#define GET_TAG(con) (get_itbl(con)->srt_bitmap)

#ifdef TABLES_NEXT_TO_CODE
#define INFO_PTR_TO_STRUCT(info) ((StgInfoTable *)(info) - 1)
#define RET_INFO_PTR_TO_STRUCT(info) ((StgRetInfoTable *)(info) - 1)
#define FUN_INFO_PTR_TO_STRUCT(info) ((StgFunInfoTable *)(info) - 1)
#define THUNK_INFO_PTR_TO_STRUCT(info) ((StgThunkInfoTable *)(info) - 1)
#define CON_INFO_PTR_TO_STRUCT(info) ((StgConInfoTable *)(info) - 1)
#define itbl_to_fun_itbl(i) ((StgFunInfoTable *)(((StgInfoTable *)(i) + 1)) - 1)
#define itbl_to_ret_itbl(i) ((StgRetInfoTable *)(((StgInfoTable *)(i) + 1)) - 1)
#define itbl_to_thunk_itbl(i) ((StgThunkInfoTable *)(((StgInfoTable *)(i) + 1)) - 1)
#define itbl_to_con_itbl(i) ((StgConInfoTable *)(((StgInfoTable *)(i) + 1)) - 1)
#else
#define INFO_PTR_TO_STRUCT(info) ((StgInfoTable *)info)
#define RET_INFO_PTR_TO_STRUCT(info) ((StgRetInfoTable *)info)
#define FUN_INFO_PTR_TO_STRUCT(info) ((StgFunInfoTable *)info)
#define THUNK_INFO_PTR_TO_STRUCT(info) ((StgThunkInfoTable *)info)
#define CON_INFO_PTR_TO_STRUCT(info) ((StgConInfoTable *)info)
#define itbl_to_fun_itbl(i) ((StgFunInfoTable *)(i))
#define itbl_to_ret_itbl(i) ((StgRetInfoTable *)(i))
#define itbl_to_thunk_itbl(i) ((StgThunkInfoTable *)(i))
#define itbl_to_con_itbl(i) ((StgConInfoTable *)(i))
#endif

/* -----------------------------------------------------------------------------
   Macros for building closures
   -------------------------------------------------------------------------- */

#ifdef PROFILING
#ifdef DEBUG_RETAINER
/* 
  For the sake of debugging, we take the safest way for the moment. Actually, this 
  is useful to check the sanity of heap before beginning retainer profiling.
  flip is defined in RetainerProfile.c, and declared as extern in RetainerProfile.h.
  Note: change those functions building Haskell objects from C datatypes, i.e.,
  all rts_mk???() functions in RtsAPI.c, as well.
 */
#define SET_PROF_HDR(c,ccs_)            \
        ((c)->header.prof.ccs = ccs_, (c)->header.prof.hp.rs = (retainerSet *)((StgWord)NULL | flip))
#else
/*
  For retainer profiling only: we do not have to set (c)->header.prof.hp.rs to
  NULL | flip (flip is defined in RetainerProfile.c) because even when flip
  is 1, rs is invalid and will be initialized to NULL | flip later when 
  the closure *c is visited.
 */
/*
#define SET_PROF_HDR(c,ccs_)            \
        ((c)->header.prof.ccs = ccs_, (c)->header.prof.hp.rs = NULL)
 */
/*
  The following macro works for both retainer profiling and LDV profiling:
  for retainer profiling, ldvTime remains 0, so rs fields are initialized to 0.
  See the invariants on ldvTime.
 */
#define SET_PROF_HDR(c,ccs_)            \
        ((c)->header.prof.ccs = ccs_,   \
        LDV_RECORD_CREATE((c)))
#endif /* DEBUG_RETAINER */
#else
#define SET_PROF_HDR(c,ccs)
#endif

#define SET_HDR(c,_info,ccs)				\
   {							\
	(c)->header.info = _info;			\
	SET_PROF_HDR((StgClosure *)(c),ccs);		\
   }

#define SET_ARR_HDR(c,info,costCentreStack,n_bytes)	\
   SET_HDR(c,info,costCentreStack);			\
   (c)->bytes = n_bytes;

// Use when changing a closure from one kind to another
#define OVERWRITE_INFO(c, new_info)                             \
    OVERWRITING_CLOSURE((StgClosure *)(c));                     \
    SET_INFO((c), (new_info));                                  \
    LDV_RECORD_CREATE(c);

/* -----------------------------------------------------------------------------
   How to get hold of the static link field for a static closure.
   -------------------------------------------------------------------------- */

/* These are hard-coded. */
#define FUN_STATIC_LINK(p)   (&(p)->payload[0])
#define THUNK_STATIC_LINK(p) (&(p)->payload[1])
#define IND_STATIC_LINK(p)   (&(p)->payload[1])

INLINE_HEADER StgClosure **
STATIC_LINK(const StgInfoTable *info, StgClosure *p)
{ 
    switch (info->type) {
    case THUNK_STATIC:
	return THUNK_STATIC_LINK(p);
    case FUN_STATIC:
	return FUN_STATIC_LINK(p);
    case IND_STATIC:
	return IND_STATIC_LINK(p);
    default:
	return &(p)->payload[info->layout.payload.ptrs +
			     info->layout.payload.nptrs];
    }
}

#define STATIC_LINK2(info,p)							\
   (*(StgClosure**)(&((p)->payload[info->layout.payload.ptrs +			\
					info->layout.payload.nptrs + 1])))

/* -----------------------------------------------------------------------------
   INTLIKE and CHARLIKE closures.
   -------------------------------------------------------------------------- */

#define CHARLIKE_CLOSURE(n) ((P_)&stg_CHARLIKE_closure[(n)-MIN_CHARLIKE])
#define INTLIKE_CLOSURE(n)  ((P_)&stg_INTLIKE_closure[(n)-MIN_INTLIKE])

/* ----------------------------------------------------------------------------
   Macros for untagging and retagging closure pointers
   For more information look at the comments in Cmm.h
   ------------------------------------------------------------------------- */

static inline StgWord
GET_CLOSURE_TAG(StgClosure * p)
{
    return (StgWord)p & TAG_MASK;
}

static inline StgClosure *
UNTAG_CLOSURE(StgClosure * p)
{
    return (StgClosure*)((StgWord)p & ~TAG_MASK);
}

static inline StgClosure *
TAG_CLOSURE(StgWord tag,StgClosure * p)
{
    return (StgClosure*)((StgWord)p | tag);
}

/* -----------------------------------------------------------------------------
   Forwarding pointers
   -------------------------------------------------------------------------- */

#define IS_FORWARDING_PTR(p) ((((StgWord)p) & 1) != 0)
#define MK_FORWARDING_PTR(p) (((StgWord)p) | 1)
#define UN_FORWARDING_PTR(p) (((StgWord)p) - 1)

/* -----------------------------------------------------------------------------
   DEBUGGING predicates for pointers

   LOOKS_LIKE_INFO_PTR(p)    returns False if p is definitely not an info ptr
   LOOKS_LIKE_CLOSURE_PTR(p) returns False if p is definitely not a closure ptr

   These macros are complete but not sound.  That is, they might
   return false positives.  Do not rely on them to distinguish info
   pointers from closure pointers, for example.

   We don't use address-space predicates these days, for portability
   reasons, and the fact that code/data can be scattered about the
   address space in a dynamically-linked environment.  Our best option
   is to look at the alleged info table and see whether it seems to
   make sense...
   -------------------------------------------------------------------------- */

INLINE_HEADER rtsBool LOOKS_LIKE_INFO_PTR_NOT_NULL (StgWord p)
{
    StgInfoTable *info = INFO_PTR_TO_STRUCT(p);
    return info->type != INVALID_OBJECT && info->type < N_CLOSURE_TYPES;
}

INLINE_HEADER rtsBool LOOKS_LIKE_INFO_PTR (StgWord p)
{
    return p && (IS_FORWARDING_PTR(p) || LOOKS_LIKE_INFO_PTR_NOT_NULL(p));
}

INLINE_HEADER rtsBool LOOKS_LIKE_CLOSURE_PTR (void *p)
{
    return LOOKS_LIKE_INFO_PTR((StgWord)(UNTAG_CLOSURE((StgClosure *)(p)))->header.info);
}

/* -----------------------------------------------------------------------------
   Macros for calculating the size of a closure
   -------------------------------------------------------------------------- */

EXTERN_INLINE StgOffset PAP_sizeW   ( nat n_args );
EXTERN_INLINE StgOffset PAP_sizeW   ( nat n_args )
{ return sizeofW(StgPAP) + n_args; }

EXTERN_INLINE StgOffset AP_sizeW   ( nat n_args );
EXTERN_INLINE StgOffset AP_sizeW   ( nat n_args )
{ return sizeofW(StgAP) + n_args; }

EXTERN_INLINE StgOffset AP_STACK_sizeW ( nat size );
EXTERN_INLINE StgOffset AP_STACK_sizeW ( nat size )
{ return sizeofW(StgAP_STACK) + size; }

EXTERN_INLINE StgOffset CONSTR_sizeW( nat p, nat np );
EXTERN_INLINE StgOffset CONSTR_sizeW( nat p, nat np )
{ return sizeofW(StgHeader) + p + np; }

EXTERN_INLINE StgOffset THUNK_SELECTOR_sizeW ( void );
EXTERN_INLINE StgOffset THUNK_SELECTOR_sizeW ( void )
{ return sizeofW(StgSelector); }

EXTERN_INLINE StgOffset BLACKHOLE_sizeW ( void );
EXTERN_INLINE StgOffset BLACKHOLE_sizeW ( void )
{ return sizeofW(StgInd); } // a BLACKHOLE is a kind of indirection

/* --------------------------------------------------------------------------
   Sizes of closures
   ------------------------------------------------------------------------*/

EXTERN_INLINE StgOffset sizeW_fromITBL( const StgInfoTable* itbl );
EXTERN_INLINE StgOffset sizeW_fromITBL( const StgInfoTable* itbl )
{ return sizeofW(StgClosure)
       + sizeofW(StgPtr)  * itbl->layout.payload.ptrs
       + sizeofW(StgWord) * itbl->layout.payload.nptrs; }

EXTERN_INLINE StgOffset thunk_sizeW_fromITBL( const StgInfoTable* itbl );
EXTERN_INLINE StgOffset thunk_sizeW_fromITBL( const StgInfoTable* itbl )
{ return sizeofW(StgThunk)
       + sizeofW(StgPtr)  * itbl->layout.payload.ptrs
       + sizeofW(StgWord) * itbl->layout.payload.nptrs; }

EXTERN_INLINE StgOffset ap_stack_sizeW( StgAP_STACK* x );
EXTERN_INLINE StgOffset ap_stack_sizeW( StgAP_STACK* x )
{ return AP_STACK_sizeW(x->size); }

EXTERN_INLINE StgOffset ap_sizeW( StgAP* x );
EXTERN_INLINE StgOffset ap_sizeW( StgAP* x )
{ return AP_sizeW(x->n_args); }

EXTERN_INLINE StgOffset pap_sizeW( StgPAP* x );
EXTERN_INLINE StgOffset pap_sizeW( StgPAP* x )
{ return PAP_sizeW(x->n_args); }

EXTERN_INLINE StgWord arr_words_words( StgArrWords* x);
EXTERN_INLINE StgWord arr_words_words( StgArrWords* x)
{ return ROUNDUP_BYTES_TO_WDS(x->bytes); }

EXTERN_INLINE StgOffset arr_words_sizeW( StgArrWords* x );
EXTERN_INLINE StgOffset arr_words_sizeW( StgArrWords* x )
{ return sizeofW(StgArrWords) + arr_words_words(x); }

EXTERN_INLINE StgOffset mut_arr_ptrs_sizeW( StgMutArrPtrs* x );
EXTERN_INLINE StgOffset mut_arr_ptrs_sizeW( StgMutArrPtrs* x )
{ return sizeofW(StgMutArrPtrs) + x->size; }

EXTERN_INLINE StgWord stack_sizeW ( StgStack *stack );
EXTERN_INLINE StgWord stack_sizeW ( StgStack *stack )
{ return sizeofW(StgStack) + stack->stack_size; }

EXTERN_INLINE StgWord bco_sizeW ( StgBCO *bco );
EXTERN_INLINE StgWord bco_sizeW ( StgBCO *bco )
{ return bco->size; }

EXTERN_INLINE nat closure_sizeW_ (StgClosure *p, StgInfoTable *info);
EXTERN_INLINE nat
closure_sizeW_ (StgClosure *p, StgInfoTable *info)
{
    switch (info->type) {
    case THUNK_0_1:
    case THUNK_1_0:
	return sizeofW(StgThunk) + 1;
    case FUN_0_1:
    case CONSTR_0_1:
    case FUN_1_0:
    case CONSTR_1_0:
	return sizeofW(StgHeader) + 1;
    case THUNK_0_2:
    case THUNK_1_1:
    case THUNK_2_0:
	return sizeofW(StgThunk) + 2;
    case FUN_0_2:
    case CONSTR_0_2:
    case FUN_1_1:
    case CONSTR_1_1:
    case FUN_2_0:
    case CONSTR_2_0:
	return sizeofW(StgHeader) + 2;
    case THUNK:
	return thunk_sizeW_fromITBL(info);
    case THUNK_SELECTOR:
	return THUNK_SELECTOR_sizeW();
    case AP_STACK:
	return ap_stack_sizeW((StgAP_STACK *)p);
    case AP:
	return ap_sizeW((StgAP *)p);
    case PAP:
	return pap_sizeW((StgPAP *)p);
    case IND:
    case IND_PERM:
	return sizeofW(StgInd);
    case ARR_WORDS:
	return arr_words_sizeW((StgArrWords *)p);
    case MUT_ARR_PTRS_CLEAN:
    case MUT_ARR_PTRS_DIRTY:
    case MUT_ARR_PTRS_FROZEN:
    case MUT_ARR_PTRS_FROZEN0:
	return mut_arr_ptrs_sizeW((StgMutArrPtrs*)p);
    case TSO:
        return sizeofW(StgTSO);
    case STACK:
        return stack_sizeW((StgStack*)p);
    case BCO:
	return bco_sizeW((StgBCO *)p);
    case TREC_CHUNK:
        return sizeofW(StgTRecChunk);
    default:
	return sizeW_fromITBL(info);
    }
}

// The definitive way to find the size, in words, of a heap-allocated closure
EXTERN_INLINE nat closure_sizeW (StgClosure *p);
EXTERN_INLINE nat closure_sizeW (StgClosure *p)
{
    return closure_sizeW_(p, get_itbl(p));
}

/* -----------------------------------------------------------------------------
   Sizes of stack frames
   -------------------------------------------------------------------------- */

EXTERN_INLINE StgWord stack_frame_sizeW( StgClosure *frame );
EXTERN_INLINE StgWord stack_frame_sizeW( StgClosure *frame )
{
    StgRetInfoTable *info;

    info = get_ret_itbl(frame);
    switch (info->i.type) {

    case RET_DYN:
    {
	StgRetDyn *dyn = (StgRetDyn *)frame;
	return  sizeofW(StgRetDyn) + RET_DYN_BITMAP_SIZE + 
	    RET_DYN_NONPTR_REGS_SIZE +
	    RET_DYN_PTRS(dyn->liveness) + RET_DYN_NONPTRS(dyn->liveness);
    }
	    
    case RET_FUN:
	return sizeofW(StgRetFun) + ((StgRetFun *)frame)->size;

    case RET_BIG:
	return 1 + GET_LARGE_BITMAP(&info->i)->size;

    case RET_BCO:
	return 2 + BCO_BITMAP_SIZE((StgBCO *)((P_)frame)[1]);

    default:
	return 1 + BITMAP_SIZE(info->i.layout.bitmap);
    }
}

/* -----------------------------------------------------------------------------
   StgMutArrPtrs macros

   An StgMutArrPtrs has a card table to indicate which elements are
   dirty for the generational GC.  The card table is an array of
   bytes, where each byte covers (1 << MUT_ARR_PTRS_CARD_BITS)
   elements.  The card table is directly after the array data itself.
   -------------------------------------------------------------------------- */

// The number of card bytes needed
INLINE_HEADER W_ mutArrPtrsCards (W_ elems)
{
    return (W_)((elems + (1 << MUT_ARR_PTRS_CARD_BITS) - 1)
                           >> MUT_ARR_PTRS_CARD_BITS);
}

// The number of words in the card table
INLINE_HEADER W_ mutArrPtrsCardTableSize (W_ elems)
{
    return ROUNDUP_BYTES_TO_WDS(mutArrPtrsCards(elems));
}

// The address of the card for a particular card number
INLINE_HEADER StgWord8 *mutArrPtrsCard (StgMutArrPtrs *a, W_ n)
{
    return ((StgWord8 *)&(a->payload[a->ptrs]) + n);
}

/* -----------------------------------------------------------------------------
   Replacing a closure with a different one.  We must call
   OVERWRITING_CLOSURE(p) on the old closure that is about to be
   overwritten.

   In PROFILING mode, LDV profiling requires that we fill the slop
   with zeroes, and record the old closure as dead (LDV_recordDead()).

   In DEBUG mode, we must overwrite the slop with zeroes, because the
   sanity checker wants to walk through the heap checking all the
   pointers.

   In multicore mode, we *cannot* overwrite slop with zeroes, because
   another thread might be reading it.  So,

      LDV PROFILING is not compatible with +RTS -N<n> (for n > 1)

      THREADED_RTS can be used with DEBUG, but full heap sanity
      checking is disabled except after major GC.

   -------------------------------------------------------------------------- */

#if defined(PROFILING) || (!defined(THREADED_RTS) && defined(DEBUG))
#define OVERWRITING_CLOSURE(c) overwritingClosure(c)
#else
#define OVERWRITING_CLOSURE(c) /* nothing */
#endif

#ifdef PROFILING
void LDV_recordDead (StgClosure *c, nat size);
#endif

EXTERN_INLINE void overwritingClosure (StgClosure *p);
EXTERN_INLINE void overwritingClosure (StgClosure *p)
{
    nat size, i;

#if defined(PROFILING)
    if (era <= 0) return;
#endif

    size = closure_sizeW(p);

    // For LDV profiling, we need to record the closure as dead
#if defined(PROFILING)
    LDV_recordDead((StgClosure *)(p), size);
#endif

    for (i = 0; i < size - sizeofW(StgThunkHeader); i++) {
        ((StgThunk *)(p))->payload[i] = 0;
    }
}

#endif /* RTS_STORAGE_CLOSUREMACROS_H */
