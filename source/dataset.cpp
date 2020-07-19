#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "dataset.h"

/* This function clears the dataset structure to all zeroes.
 * It does not do memory release; it is meant to initialise the structure prior first use.
*/
void dataset_reset(DATASET &dataset)
{
    /*dataset.nan=0;
    dataset.val=0;
    dataset.val_int=0;*/
    dataset.sx=dataset.sy=0;
    dataset.data.clear();
}


/*
 * Transpose a matrix of data of size sx, sy.
 * Allocates space for the new matrix with malloc - to be freed with free.
 * Noes not deallocate the original data.
*/
double *transpose(double *data,unsigned sx,unsigned sy)
{
    // Allocate memory
    double *t = (double*)malloc(sx*sy*sizeof(double));
    if(t==0)
        return t;
    // Do the transpose (the initial order of the data is irrelevant; the operation is identical)
    for(unsigned y=0;y<sy;y++)
    {
        for(unsigned x=0;x<sx;x++)
        {
            t[x*sy+y]=data[y*sx+x];
        }
    }


    return t;
}
bool *transpose(bool *data,unsigned sx,unsigned sy)
{
    // Allocate memory
    bool *t = (bool*)malloc(sx*sy*sizeof(bool));
    if(t==0)
        return t;
    // Do the transpose (the initial order of the data is irrelevant; the operation is identical)
    for(unsigned y=0;y<sy;y++)
    {
        for(unsigned x=0;x<sx;x++)
        {
            t[x*sy+y]=data[y*sx+x];
        }
    }


    return t;
}
/* Allocate memory and convers the double data to integer.
*/
int *double_to_int(double *data,unsigned sx,unsigned sy)
{
    // Allocate memory
    int *t = (int*)malloc(sx*sy*sizeof(int));
    if(t==0)
        return t;
    // Convert
    for(unsigned i=0;i<sx*sy;i++)
    {
        t[i] = data[i];
    }
    return t;
}


// Count number of instances
unsigned datasetCountInstances(DATASET &dataset, std::vector<int> &nulllabel, unsigned labelchannel)
{
    // Convers the label channel into a label data structure
    assert(dataset.sx>1); assert(dataset.sy>1);
    assert(labelchannel<dataset.sx);

    LABEL l;
    unsigned numinst=0;

    bool doinit=true;
    for(unsigned i=1;i<dataset.sy;i++)   // Start at 1; plays with i-1 in doinit
    {
        if(doinit)
        {
            l.from = i-1;
            l.label = dataset.data[labelchannel][i-1];
            doinit=false;
        }
        if(dataset.data[labelchannel][i] != l.label)
        {
            // label has changed -> add start-end
            l.to = i-1;
            // Only add the label if it is not a NULL label
            if( std::find(nulllabel.begin(), nulllabel.end(), l.label) == nulllabel.end() )
            {
                //printf("Adding %d-%d %d in main loop\n",l.from,l.to,l.label);
                numinst++;
            }
            // Reinit for next label
            doinit=true;
        }
    }
    // If doinit is still set it means a label changed on the last sample. We lose the last sample.
    if(!doinit)
    {
        l.to = dataset.sy-1;
        // Only add the label if it is not a NULL label
        if( std::find(nulllabel.begin(), nulllabel.end(), l.label) == nulllabel.end() )
        {
            //printf("Adding %d-%d %d at end\n",l.from,l.to,l.label);
            numinst++;
        }
    }

    return numinst;

    //printLabelStructure();
}


