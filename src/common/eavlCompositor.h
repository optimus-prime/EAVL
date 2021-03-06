// Copyright 2010-2013 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_COMPOSITOR_H
#define EAVL_COMPOSITOR_H

#include "eavlConfig.h"
#ifdef HAVE_MPI
#include "mpi.h"
#include <boost/mpi.hpp>

void ParallelZComposite(boost::mpi::communicator &comm,
                        int npixels,
                        const float *inz, const unsigned char *inrgba,
                        float *outz, unsigned char *outrgba,
                        unsigned char bgr, unsigned char bgg, unsigned char bgb);

 
#endif

#endif
