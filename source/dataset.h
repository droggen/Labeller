#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <string>

typedef struct {
    unsigned sx,sy;
    std::vector<std::vector<int> > data;
    std::string filename,filename_nopath,filename_path;

    //std::vector<int>
} DATASET;

typedef struct {
    unsigned from,to;
    int label;
} LABEL;

// Information about whether an instance is grabed, and from which extremity
typedef struct {
    int instance;
    bool isfrom;
    int at;
} INSTANCEGRABBED;

double *transpose(double *data,unsigned sx,unsigned sy);
bool *transpose(bool *data,unsigned sx,unsigned sy);

int *double_to_int(double *data,unsigned sx,unsigned sy);

void dataset_reset(DATASET &dataset);

unsigned datasetCountInstances(DATASET &dataset,std::vector<int> &nulllabel,unsigned labelchannel);



#endif // DATASET_H
