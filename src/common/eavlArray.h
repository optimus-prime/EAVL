// Copyright 2010-2012 UT-Battelle, LLC.  See LICENSE.txt for more information.
#ifndef EAVL_ARRAY_H
#define EAVL_ARRAY_H

#include "STL.h"
#include "eavl.h"
#include "eavlException.h"
#include "eavlCUDA.h"

#ifdef HAVE_CUDA
#include <cuda.h>
#include <cuda_runtime_api.h>
#endif

#include <cfloat>


// ****************************************************************************
// Class:  eavlArray
//
// Purpose:
///   Abstract base class for a variable-length, multiple-component array
///   of primitive types.  Subclasses handle specific types.
///   \todo: Maybe we DON'T need a name in here?  Upside is it's associated
///   most directly with the data.  Downside is that most things that use
///   arrays may want their own name anyway, e.g. a map<> in eavlField
///   or something.....
//
// Programmer:  Jeremy Meredith, Dave Pugmire, Sean Ahern
// Creation:    February 14, 2011
//
// Modifications:
// ****************************************************************************
class eavlArray
{
  protected:
    string        name;
    int           ncomponents;
  public:
    eavlArray(const string &n,     ///< name
              int nc = 1)          ///< number of components
        : name(n)
    {
        SetNumberOfComponents(nc);
    }
    virtual ~eavlArray() { }
    const string &GetName()
    {
        return name;
    }
    virtual const char *GetBasicType() = 0;
    virtual void   SetNumberOfTuples(int) = 0;
    virtual int    GetNumberOfTuples() const = 0;
    virtual double GetComponentAsDouble(
                                      int i,  ///< tuple index
                                      int c   ///< component index
                                      ) = 0;
    virtual void   SetComponentFromDouble(int i, int c, double v) = 0;

    enum Location { HOST, DEVICE };
#ifdef HAVE_CUDA
    virtual void *GetCUDAArray() = 0;
#else
    virtual void *GetCUDAArray() {THROW(eavlException,"CUDA not available");}
#endif
    virtual void *GetHostArray() = 0;
    void *GetRawPointer(Location loc)
    {
        if (loc == HOST)
            return GetHostArray();
        else
            return GetCUDAArray();
    }

    virtual long long GetMemoryUsage()
    {
        return sizeof(string) + name.size()*sizeof(char) + sizeof(int);
    }
    int GetNumberOfComponents()
    {
        return ncomponents;
    }
    double GetMaxAsDouble(int comp = -1)
    {
        int nc = GetNumberOfComponents();
        int nt = GetNumberOfTuples();
        double mymax = -DBL_MAX;
        if (comp < 0)
        {
            for (int i=0; i<nt; i++)
            {
                for (int j=0; j<nc; j++)
                {
                    double v = GetComponentAsDouble(i,j);
                    if (v > mymax)
                        mymax = v;
                }
            }
        }
        else
        {
            for (int i=0; i<nt; i++)
            {
                double v = GetComponentAsDouble(i,comp);
                if (v > mymax)
                    mymax = v;
            }
        }
        return mymax;
    }
    double GetMinAsDouble(int comp = -1)
    {
        int nc = GetNumberOfComponents();
        int nt = GetNumberOfTuples();
        double mymax = +DBL_MAX;
        if (comp < 0)
        {
            for (int i=0; i<nt; i++)
            {
                for (int j=0; j<nc; j++)
                {
                    double v = GetComponentAsDouble(i,j);
                    if (v < mymax)
                        mymax = v;
                }
            }
        }
        else
        {
            for (int i=0; i<nt; i++)
            {
                double v = GetComponentAsDouble(i,comp);
                if (v < mymax)
                    mymax = v;
            }
        }
        return mymax;
    }
    void PrintSummary(ostream &out)
    {
        int n = GetNumberOfTuples() * GetNumberOfComponents();
        out << GetBasicType() <<" "
            << GetName()
            <<"["<< GetNumberOfTuples() <<"]"
            <<"["<< GetNumberOfComponents()<<"] = ";
        if (n == 0)
            out << "(empty)";
        for (int i=0; i<n; i++)
        {
            if (n <= 11)
                out << GetComponentAsDouble(i/ncomponents,i%ncomponents) << "  ";
            else
            {
                out << GetComponentAsDouble(i/ncomponents,i%ncomponents);
                if (i==0 || i==1 || i==2 || i==3 || i==n-5 || i==n-4 || i==n-3 || i==n-2)
                {
                    out << "  ";
                }
                else if (i == 4)
                {
                    out << " ... ";
                    i = n-6;
                }
            }
        }
        out << endl;
    }
  private:
    void SetNumberOfComponents(int nc)
    {
        ncomponents = nc;
    }
};

// ****************************************************************************
// Class:  eavlFloatArray
//
// Purpose:
///   Concrete subclass of eavlArray.
//
// Programmer:  Jeremy Meredith, Dave Pugmire, Sean Ahern
// Creation:    February 14, 2011
//
// ****************************************************************************
class eavlFloatArray : public eavlArray
{
  protected:
    vector<float> values;
#ifdef HAVE_CUDA
    bool    host_dirty;
    bool    device_dirty;
    float *device_values;
    void NeedToUseOnHost()
    {
        if (device_dirty)
        {
            int nbytes = values.size() * sizeof(float);
            cudaMemcpy(&(values[0]), device_values,
                       nbytes, cudaMemcpyDeviceToHost);
            CUDA_CHECK_ERROR();
        }
        device_dirty = false;
        host_dirty = true;
    }
    void NeedToUseOnDevice()
    {
        int nbytes = values.size() * sizeof(float);
        if (device_values == NULL)
        {
            cudaMalloc((void**)&device_values, nbytes);
            CUDA_CHECK_ERROR();
            //cudaMemset((void**)&device_values, 0, nbytes);
        }
        if (host_dirty)
        {
            cudaMemcpy(device_values, &(values[0]),
                       nbytes, cudaMemcpyHostToDevice);
            CUDA_CHECK_ERROR();
        }
        host_dirty = false;
        device_dirty = true;
    }
#else
    void NeedToUseOnHost() const {};
    void NeedToUseOnDevice() const {};
#endif
  public:
    eavlFloatArray(const string &n, int nc = 1, int nt = 0) : eavlArray(n,nc)
    {
#ifdef HAVE_CUDA
        host_dirty = false;
        device_dirty = false;
        device_values = NULL;
#endif
        if (nt > 0)
            values.resize(ncomponents * nt);
    }
    virtual ~eavlFloatArray()
    {
#ifdef HAVE_CUDA
        if (device_values)
            cudaFree(device_values);
#endif
    }
    virtual const char *GetBasicType() { return "float"; }
    virtual void *GetHostArray()
    {
        NeedToUseOnHost();
        return &(values[0]);
    }
    virtual void SetNumberOfTuples(int n)
    {
        NeedToUseOnHost();
        values.resize(ncomponents * n);
    }
    virtual int GetNumberOfTuples() const
    {
        //NeedToUseOnHost();
        if (ncomponents == 0)
            return 0;
        return values.size() / ncomponents;
    }
    void SetTuple(int index, float *v)
    {
        NeedToUseOnHost();
        for (int c=0; c<ncomponents; c++)
            values[index*ncomponents+c] = v[c];
    }
    ///\todo: there should be a const version of GetTuple, right? (hard with cuda added)
#ifndef HAVE_CUDA
    const float *GetTuple(int index) const
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
#endif
    float *GetTuple(int index)
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
    float GetValue(int index)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        return values[index*ncomponents+0];
    }
    void SetValue(int index, float v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values[index*ncomponents+0] = v;
    }
    void AddValue(float v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values.push_back(v);
    }
    virtual double GetComponentAsDouble(int i, int c)
    {
        NeedToUseOnHost();
        return GetTuple(i)[c];
    }
    virtual void SetComponentFromDouble(int i, int c, double v)
    {
        NeedToUseOnHost();
        GetTuple(i)[c] = v;
    }
#ifdef HAVE_CUDA
    virtual void *GetCUDAArray()
    {
        NeedToUseOnDevice();
        return (void*)(device_values);
    }
    void ZeroHostArrayForDebugging()
    {
        for (size_t i=0; i<values.size(); i++)
            values[i] = 0;
    }
#endif
    virtual long long GetMemoryUsage()
    {
        ///\todo: ignores device memory; is that right??
        long long mem = 0;

        mem += sizeof(vector<float>);
        mem += values.size() * sizeof(float);

#ifdef HAVE_CUDA
        mem += sizeof(bool);
        mem += sizeof(bool);
        mem += sizeof(float*);
#endif
        return mem + eavlArray::GetMemoryUsage();
    }
};



// ****************************************************************************
// Class:  eavlIntArray
//
// Purpose:
///   Concrete subclass of eavlArray.
//
// Programmer:  Jeremy Meredith, Dave Pugmire, Sean Ahern
// Creation:    February 14, 2011
//
// ****************************************************************************
class eavlIntArray : public eavlArray
{
  protected:
    vector<int> values;
#ifdef HAVE_CUDA
    bool    host_dirty;
    bool    device_dirty;
    int *device_values;
    void NeedToUseOnHost()
    {
        if (device_dirty)
        {
            int nbytes = values.size() * sizeof(int);
            cudaMemcpy(&(values[0]), device_values,
                       nbytes, cudaMemcpyDeviceToHost);
            CUDA_CHECK_ERROR();
        }
        device_dirty = false;
        host_dirty = true;
    }
    void NeedToUseOnDevice()
    {
        int nbytes = values.size() * sizeof(int);
        if (device_values == NULL)
        {
            cudaMalloc((void**)&device_values, nbytes);
            CUDA_CHECK_ERROR();
            //cudaMemset((void**)&device_values, 0, nbytes);
        }
        if (host_dirty)
        {
            cudaMemcpy(device_values, &(values[0]),
                       nbytes, cudaMemcpyHostToDevice);
            CUDA_CHECK_ERROR();
        }
        host_dirty = false;
        device_dirty = true;
    }
#else
    void NeedToUseOnHost() const {};
    void NeedToUseOnDevice() const {};
#endif
  public:
    eavlIntArray(const string &n, int nc = 1, int nt = 0) : eavlArray(n,nc)
    {
#ifdef HAVE_CUDA
        host_dirty = false;
        device_dirty = false;
        device_values = NULL;
#endif
        if (nt > 0)
            values.resize(ncomponents * nt);
    }
    virtual ~eavlIntArray()
    {
#ifdef HAVE_CUDA
        if (device_values)
            cudaFree(device_values);
#endif
    }
    virtual const char *GetBasicType() { return "int"; }
    virtual void *GetHostArray()
    {
        NeedToUseOnHost();
        return &(values[0]);
    }
    virtual void SetNumberOfTuples(int n)
    {
        NeedToUseOnHost();
        values.resize(ncomponents * n);
    }
    virtual int GetNumberOfTuples() const
    {
        //NeedToUseOnHost();
        if (ncomponents == 0)
            return 0;
        return values.size() / ncomponents;
    }
    void SetTuple(int index, int *v)
    {
        NeedToUseOnHost();
        for (int c=0; c<ncomponents; c++)
            values[index*ncomponents+c] = v[c];
    }
    ///\todo: there should be a const version of GetTuple, right? (hard with cuda added)
#ifndef HAVE_CUDA
    const int *GetTuple(int index) const
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
#endif
    int *GetTuple(int index)
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
    int GetValue(int index)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        return values[index*ncomponents+0];
    }
    void SetValue(int index, int v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values[index*ncomponents+0] = v;
    }
    void AddValue(int v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values.push_back(v);
    }
    virtual double GetComponentAsDouble(int i, int c)
    {
        NeedToUseOnHost();
        return GetTuple(i)[c];
    }
    virtual void SetComponentFromDouble(int i, int c, double v)
    {
        NeedToUseOnHost();
        GetTuple(i)[c] = v;
    }
#ifdef HAVE_CUDA
    virtual void *GetCUDAArray()
    {
        NeedToUseOnDevice();
        return (void*)(device_values);
    }
    void ZeroHostArrayForDebugging()
    {
        for (size_t i=0; i<values.size(); i++)
            values[i] = 0;
    }
#endif
    virtual long long GetMemoryUsage()
    {
        ///\todo: ignores device memory; is that right??
        long long mem = 0;

        mem += sizeof(vector<int>);
        mem += values.size() * sizeof(int);

#ifdef HAVE_CUDA
        mem += sizeof(bool);
        mem += sizeof(bool);
        mem += sizeof(int*);
#endif
        return mem + eavlArray::GetMemoryUsage();
    }
};


// ****************************************************************************
// Class:  eavlByteArray
//
// Purpose:
///   Concrete subclass of eavlArray.
//
// Programmer:  Jeremy Meredith, Dave Pugmire, Sean Ahern
// Creation:    February 14, 2011
//
// ****************************************************************************
class eavlByteArray : public eavlArray
{
  protected:
    vector<byte> values;
#ifdef HAVE_CUDA
    bool    host_dirty;
    bool    device_dirty;
    byte *device_values;
    void NeedToUseOnHost()
    {
        if (device_dirty)
        {
            int nbytes = values.size() * sizeof(byte);
            cudaMemcpy(&(values[0]), device_values,
                       nbytes, cudaMemcpyDeviceToHost);
            CUDA_CHECK_ERROR();
        }
        device_dirty = false;
        host_dirty = true;
    }
    void NeedToUseOnDevice()
    {
        int nbytes = values.size() * sizeof(byte);
        if (device_values == NULL)
        {
            cudaMalloc((void**)&device_values, nbytes);
            CUDA_CHECK_ERROR();
            //cudaMemset((void**)&device_values, 0, nbytes);
        }
        if (host_dirty)
        {
            cudaMemcpy(device_values, &(values[0]),
                       nbytes, cudaMemcpyHostToDevice);
            CUDA_CHECK_ERROR();
        }
        host_dirty = false;
        device_dirty = true;
    }
#else
    void NeedToUseOnHost() const {};
    void NeedToUseOnDevice() const {};
#endif
  public:
    eavlByteArray(const string &n, int nc = 1, int nt = 0) : eavlArray(n,nc)
    {
#ifdef HAVE_CUDA
        host_dirty = false;
        device_dirty = false;
        device_values = NULL;
#endif
        if (nt > 0)
            values.resize(ncomponents * nt);
    }
    virtual ~eavlByteArray()
    {
#ifdef HAVE_CUDA
        if (device_values)
            cudaFree(device_values);
#endif
    }
    virtual const char *GetBasicType() { return "byte"; }
    virtual void *GetHostArray()
    {
        NeedToUseOnHost();
        return &(values[0]);
    }
    virtual void SetNumberOfTuples(int n)
    {
        NeedToUseOnHost();
        values.resize(ncomponents * n);
    }
    virtual int GetNumberOfTuples() const
    {
        //NeedToUseOnHost();
        if (ncomponents == 0)
            return 0;
        return values.size() / ncomponents;
    }
    void SetTuple(int index, byte *v)
    {
        NeedToUseOnHost();
        for (int c=0; c<ncomponents; c++)
            values[index*ncomponents+c] = v[c];
    }
    ///\todo: there should be a const version of GetTuple, right? (hard with cuda added)
#ifndef HAVE_CUDA
    const byte *GetTuple(int index) const
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
#endif
    byte *GetTuple(int index)
    {
        NeedToUseOnHost();
        return &(values[index*ncomponents]);
    }
    byte GetValue(int index)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        return values[index*ncomponents+0];
    }
    void SetValue(int index, byte v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values[index*ncomponents+0] = v;
    }
    void AddValue(byte v)
    {
        // assert ncomponents==1?
        NeedToUseOnHost();
        values.push_back(v);
    }
    virtual double GetComponentAsDouble(int i, int c)
    {
        NeedToUseOnHost();
        return GetTuple(i)[c];
    }
    virtual void SetComponentFromDouble(int i, int c, double v)
    {
        NeedToUseOnHost();
        GetTuple(i)[c] = v;
    }
#ifdef HAVE_CUDA
    virtual void *GetCUDAArray()
    {
        NeedToUseOnDevice();
        return (void*)(device_values);
    }
    void ZeroHostArrayForDebugging()
    {
        for (size_t i=0; i<values.size(); i++)
            values[i] = 0;
    }
#endif
    virtual long long GetMemoryUsage()
    {
        ///\todo: ignores device memory; is that right??
        long long mem = 0;

        mem += sizeof(vector<byte>);
        mem += values.size() * sizeof(byte);

#ifdef HAVE_CUDA
        mem += sizeof(bool);
        mem += sizeof(bool);
        mem += sizeof(byte*);
#endif
        return mem + eavlArray::GetMemoryUsage();
    }
};





#endif
