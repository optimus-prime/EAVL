// Copyright 2010-2012 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_OP_DISPATCH_2_1_H
#define EAVL_OP_DISPATCH_2_1_H

#include "eavlException.h"

// ----------------------------------------------------------------------------

template <template <typename KF, typename KI0, typename KI1, typename KO0> class K,
          class F,
          class S, class I0, class I1, class O0>
void eavlDispatch_2_1_final(int n, eavlArray::Location loc,
                                S &structure,
                                I0  *i0, int i0div, int i0mod, int i0mul, int i0add,
                                I1  *i1, int i1div, int i1mod, int i1mul, int i1add,
                                O0  *o0, int o0mul, int o0add,
                                F &functor)
{
    K<F,I0,I1,O0>::call(n, structure,
                        i0, i0div, i0mod, i0mul, i0add,
                        i1, i1div, i1mod, i1mul, i1add,
                        o0, o0mul, o0add,
                        functor);
}

template <template <typename KF, typename KI0, typename KI1, typename KO0> class K,
          class F,
          class S, class I0, class I1>
void eavlDispatch_2_1_stage3(int n, eavlArray::Location loc,
                                 S &structure,
                                 I0  *i0, int i0div, int i0mod, int i0mul, int i0add,
                                 I1  *i1, int i1div, int i1mod, int i1mul, int i1add,
                                 eavlArray *o0, int o0mul, int o0add,
                                 F &functor)
{
    eavlFloatArray  *o0_f = dynamic_cast<eavlFloatArray*>(o0);
    eavlByteArray   *o0_b = dynamic_cast<eavlByteArray*>(o0);
    eavlIntArray    *o0_i = dynamic_cast<eavlIntArray*>(o0);

    if (o0_f)
        eavlDispatch_2_1_final<K>(n, loc, structure,
                                       i0, i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       (float*)o0_f->GetRawPointer(loc), o0mul, o0add,
                                       functor);
    else if (o0_b)
        eavlDispatch_2_1_final<K>(n, loc, structure, 
                                       i0, i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       (byte*)o0_b->GetRawPointer(loc), o0mul, o0add,
                                       functor);
    else if (o0_i)
        eavlDispatch_2_1_final<K>(n, loc, structure,
                                       i0, i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       (int*)o0_i->GetRawPointer(loc), o0mul, o0add,
                                       functor);
    else
        THROW(eavlException,"Unknown array type");

}

template <template <typename KF, typename KI0, typename KI1, typename KO0> class K,
          class F,
          class S, class I0>
void eavlDispatch_2_1_stage2(int n, eavlArray::Location loc,
                                 S &structure,
                                 I0 *i0, int i0div, int i0mod, int i0mul, int i0add,
                                 eavlArray *i1, int i1div, int i1mod, int i1mul, int i1add,
                                 eavlArray *o0, int o0mul, int o0add,
                                 F &functor)
{
    eavlFloatArray  *i1_f = dynamic_cast<eavlFloatArray*>(i1);
    eavlByteArray   *i1_b = dynamic_cast<eavlByteArray*>(i1);
    eavlIntArray    *i1_i = dynamic_cast<eavlIntArray*>(i1);

    if (i1_f)
        eavlDispatch_2_1_stage3<K>(n, loc, structure,
                                       i0, i0div, i0mod, i0mul, i0add,
                                       (float*)i1_f->GetRawPointer(loc), i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else if (i1_b)
        eavlDispatch_2_1_stage3<K>(n, loc, structure, 
                                       i0, i0div, i0mod, i0mul, i0add,
                                       (byte*)i1_b->GetRawPointer(loc), i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else if (i1_i)
        eavlDispatch_2_1_stage3<K>(n, loc, structure,
                                       i0, i0div, i0mod, i0mul, i0add,
                                       (int*)i1_i->GetRawPointer(loc), i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else
        THROW(eavlException,"Unknown array type");
};


template <template <typename KF, typename KI0, typename KI1, typename KO0> class K,
          class F,
          class S>
void eavlDispatch_2_1(int n, eavlArray::Location loc,
                          S &structure,
                          eavlArray *i0, int i0div, int i0mod, int i0mul, int i0add,
                          eavlArray *i1, int i1div, int i1mod, int i1mul, int i1add,
                          eavlArray *o0, int o0mul, int o0add,
                          F &functor)
{
    eavlFloatArray  *i0_f = dynamic_cast<eavlFloatArray*>(i0);
    eavlByteArray   *i0_b = dynamic_cast<eavlByteArray*>(i0);
    eavlIntArray    *i0_i = dynamic_cast<eavlIntArray*>(i0);


    if (i0_f)
        eavlDispatch_2_1_stage2<K>(n, loc, structure,
                                       (float*)i0_f->GetRawPointer(loc), i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else if (i0_b)
        eavlDispatch_2_1_stage2<K>(n, loc, structure, 
                                       (byte*)i0_b->GetRawPointer(loc), i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else if (i0_i)
        eavlDispatch_2_1_stage2<K>(n, loc, structure,
                                       (int*)i0_i->GetRawPointer(loc), i0div, i0mod, i0mul, i0add,
                                       i1, i1div, i1mod, i1mul, i1add,
                                       o0, o0mul, o0add,
                                       functor);
    else
        THROW(eavlException,"Unknown array type");
};

#endif
