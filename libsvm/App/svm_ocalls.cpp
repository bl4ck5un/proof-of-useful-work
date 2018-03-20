//
// Created by fanz on 11/11/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include "svm_ocalls.h"
#include "Log.h"

int svm_save_model(const char *model_file_name, const svm_model *model)
{
    FILE *fp = fopen(model_file_name,"w");
    if(fp==NULL) return -1;

    char *old_locale = setlocale(LC_ALL, NULL);
    if (old_locale) {
        old_locale = strdup(old_locale);
    }
    setlocale(LC_ALL, "C");

    const svm_parameter& param = model->param;

    fprintf(fp,"svm_type %s\n", svm_type_table[param.svm_type]);
    fprintf(fp,"kernel_type %s\n", kernel_type_table[param.kernel_type]);

    if(param.kernel_type == POLY)
        fprintf(fp,"degree %d\n", param.degree);

    if(param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID)
        fprintf(fp,"gamma %g\n", param.gamma);

    if(param.kernel_type == POLY || param.kernel_type == SIGMOID)
        fprintf(fp,"coef0 %g\n", param.coef0);

    int nr_class = model->nr_class;
    int l = model->l;
    fprintf(fp, "nr_class %d\n", nr_class);
    fprintf(fp, "total_sv %d\n",l);

    {
        fprintf(fp, "rho");
        for(int i=0;i<nr_class*(nr_class-1)/2;i++)
            fprintf(fp," %g",model->rho[i]);
        fprintf(fp, "\n");
    }

    if(model->label)
    {
        fprintf(fp, "label");
        for(int i=0;i<nr_class;i++)
            fprintf(fp," %d",model->label[i]);
        fprintf(fp, "\n");
    }

    if(model->probA) // regression has probA only
    {
        fprintf(fp, "probA");
        for(int i=0;i<nr_class*(nr_class-1)/2;i++)
            fprintf(fp," %g",model->probA[i]);
        fprintf(fp, "\n");
    }
    if(model->probB)
    {
        fprintf(fp, "probB");
        for(int i=0;i<nr_class*(nr_class-1)/2;i++)
            fprintf(fp," %g",model->probB[i]);
        fprintf(fp, "\n");
    }

    if(model->nSV)
    {
        fprintf(fp, "nr_sv");
        for(int i=0;i<nr_class;i++)
            fprintf(fp," %d",model->nSV[i]);
        fprintf(fp, "\n");
    }

    fprintf(fp, "SV\n");
    const double * const *sv_coef = model->sv_coef;
    const svm_node * const *SV = model->SV;

    for(int i=0;i<l;i++)
    {
        for(int j=0;j<nr_class-1;j++)
            fprintf(fp, "%.16g ",sv_coef[j][i]);

        const svm_node *p = SV[i];

        if(param.kernel_type == PRECOMPUTED)
            fprintf(fp,"0:%d ",(int)(p->value));
        else
            while(p->index != -1)
            {
                fprintf(fp,"%d:%.8g ",p->index,p->value);
                p++;
            }
        fprintf(fp, "\n");
    }

    setlocale(LC_ALL, old_locale);
    free(old_locale);

    if (ferror(fp) != 0 || fclose(fp) != 0) return -1;
    else return 0;
}

static char *line = NULL;
static int max_line_len;

static char* readline(FILE *input)
{
    int len;

    if(fgets(line,max_line_len,input) == NULL)
        return NULL;

    while(strrchr(line,'\n') == NULL)
    {
        max_line_len *= 2;
        line = (char *) realloc(line,max_line_len);
        len = (int) strlen(line);
        if(fgets(line+len,max_line_len-len,input) == NULL)
            break;
    }
    return line;
}

//
// FSCANF helps to handle fscanf failures.
// Its do-while block avoids the ambiguity when
// if (...)
//    FSCANF();
// is used
//
#define FSCANF(_stream, _format, _var) do{ if (fscanf(_stream, _format, _var) != 1) return false; }while(0)
bool read_model_header(FILE *fp, svm_model* model)
{
    svm_parameter& param = model->param;
    char cmd[81];
    while(1)
    {
        FSCANF(fp,"%80s",cmd);

        if(strcmp(cmd,"svm_type")==0)
        {
            FSCANF(fp,"%80s",cmd);
            int i;
            for(i=0;svm_type_table[i];i++)
            {
                if(strcmp(svm_type_table[i],cmd)==0)
                {
                    param.svm_type=i;
                    break;
                }
            }
            if(svm_type_table[i] == NULL)
            {
                fprintf(stderr,"unknown svm type.\n");
                return false;
            }
        }
        else if(strcmp(cmd,"kernel_type")==0)
        {
            FSCANF(fp,"%80s",cmd);
            int i;
            for(i=0;kernel_type_table[i];i++)
            {
                if(strcmp(kernel_type_table[i],cmd)==0)
                {
                    param.kernel_type=i;
                    break;
                }
            }
            if(kernel_type_table[i] == NULL)
            {
                fprintf(stderr,"unknown kernel function.\n");
                return false;
            }
        }
        else if(strcmp(cmd,"degree")==0)
            FSCANF(fp,"%d",&param.degree);
        else if(strcmp(cmd,"gamma")==0)
            FSCANF(fp,"%lf",&param.gamma);
        else if(strcmp(cmd,"coef0")==0)
            FSCANF(fp,"%lf",&param.coef0);
        else if(strcmp(cmd,"nr_class")==0)
            FSCANF(fp,"%d",&model->nr_class);
        else if(strcmp(cmd,"total_sv")==0)
            FSCANF(fp,"%d",&model->l);
        else if(strcmp(cmd,"rho")==0)
        {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->rho = Malloc(double,n);
            for(int i=0;i<n;i++)
                FSCANF(fp,"%lf",&model->rho[i]);
        }
        else if(strcmp(cmd,"label")==0)
        {
            int n = model->nr_class;
            model->label = Malloc(int,n);
            for(int i=0;i<n;i++)
                FSCANF(fp,"%d",&model->label[i]);
        }
        else if(strcmp(cmd,"probA")==0)
        {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->probA = Malloc(double,n);
            for(int i=0;i<n;i++)
                FSCANF(fp,"%lf",&model->probA[i]);
        }
        else if(strcmp(cmd,"probB")==0)
        {
            int n = model->nr_class * (model->nr_class-1)/2;
            model->probB = Malloc(double,n);
            for(int i=0;i<n;i++)
                FSCANF(fp,"%lf",&model->probB[i]);
        }
        else if(strcmp(cmd,"nr_sv")==0)
        {
            int n = model->nr_class;
            model->nSV = Malloc(int,n);
            for(int i=0;i<n;i++)
                FSCANF(fp,"%d",&model->nSV[i]);
        }
        else if(strcmp(cmd,"SV")==0)
        {
            while(1)
            {
                int c = getc(fp);
                if(c==EOF || c=='\n') break;
            }
            break;
        }
        else
        {
            fprintf(stderr,"unknown text in model file: [%s]\n",cmd);
            return false;
        }
    }

    return true;

}

svm_model *svm_load_model(const char *model_file_name)
{
    FILE *fp = fopen(model_file_name,"rb");
    if(fp==NULL) return NULL;

    char *old_locale = setlocale(LC_ALL, NULL);
    if (old_locale) {
        old_locale = strdup(old_locale);
    }
    setlocale(LC_ALL, "C");

    // read parameters

    svm_model *model = Malloc(svm_model,1);
    model->rho = NULL;
    model->probA = NULL;
    model->probB = NULL;
    model->sv_indices = NULL;
    model->label = NULL;
    model->nSV = NULL;

    // read header
    if (!read_model_header(fp, model))
    {
        fprintf(stderr, "ERROR: fscanf failed to read model\n");
        setlocale(LC_ALL, old_locale);
        free(old_locale);
        free(model->rho);
        free(model->label);
        free(model->nSV);
        free(model);
        return NULL;
    }

    // read sv_coef and SV

    int elements = 0;
    long pos = ftell(fp);

    max_line_len = 1024;
    line = Malloc(char,max_line_len);
    char *p,*endptr,*idx,*val;

    while(readline(fp)!=NULL)
    {
        p = strtok(line,":");
        while(1)
        {
            p = strtok(NULL,":");
            if(p == NULL)
                break;
            ++elements;
        }
    }
    elements += model->l;

    fseek(fp,pos,SEEK_SET);

    int m = model->nr_class - 1;
    int l = model->l;
    model->sv_coef = Malloc(double *,m);
    int i;
    for(i=0;i<m;i++)
        model->sv_coef[i] = Malloc(double,l);
    model->SV = Malloc(svm_node*,l);
    svm_node *x_space = NULL;
    if(l>0) x_space = Malloc(svm_node,elements);

    int j=0;
    for(i=0;i<l;i++)
    {
        readline(fp);
        model->SV[i] = &x_space[j];

        p = strtok(line, " \t");
        model->sv_coef[0][i] = strtod(p,&endptr);
        for(int k=1;k<m;k++)
        {
            p = strtok(NULL, " \t");
            model->sv_coef[k][i] = strtod(p,&endptr);
        }

        while(1)
        {
            idx = strtok(NULL, ":");
            val = strtok(NULL, " \t");

            if(val == NULL)
                break;
            x_space[j].index = (int) strtol(idx,&endptr,10);
            x_space[j].value = strtod(val,&endptr);

            ++j;
        }
        x_space[j++].index = -1;
    }
    free(line);

    setlocale(LC_ALL, old_locale);
    free(old_locale);

    if (ferror(fp) != 0 || fclose(fp) != 0)
        return NULL;

    model->free_sv = 1;	// XXX
    return model;
}

void svm_free_model_content(svm_model* model_ptr)
{
    if(model_ptr->free_sv && model_ptr->l > 0 && model_ptr->SV != NULL)
        free((void *)(model_ptr->SV[0]));
    if(model_ptr->sv_coef)
    {
        for(int i=0;i<model_ptr->nr_class-1;i++)
            free(model_ptr->sv_coef[i]);
    }

    free(model_ptr->SV);
    model_ptr->SV = NULL;

    free(model_ptr->sv_coef);
    model_ptr->sv_coef = NULL;

    free(model_ptr->rho);
    model_ptr->rho = NULL;

    free(model_ptr->label);
    model_ptr->label= NULL;

    free(model_ptr->probA);
    model_ptr->probA = NULL;

    free(model_ptr->probB);
    model_ptr->probB= NULL;

    free(model_ptr->sv_indices);
    model_ptr->sv_indices = NULL;

    free(model_ptr->nSV);
    model_ptr->nSV = NULL;
}

void svm_free_and_destroy_model(svm_model** model_ptr_ptr)
{
    if(model_ptr_ptr != NULL && *model_ptr_ptr != NULL)
    {
        svm_free_model_content(*model_ptr_ptr);
        free(*model_ptr_ptr);
        *model_ptr_ptr = NULL;
    }
}

int svm_check_probability_model(const svm_model *model)
{
    return ((model->param.svm_type == C_SVC || model->param.svm_type == NU_SVC) &&
            model->probA!=NULL && model->probB!=NULL) ||
           ((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
            model->probA!=NULL);
}

int svm_get_svm_type(const svm_model *model)
{
    return model->param.svm_type;
}

int svm_get_nr_class(const svm_model *model)
{
    return model->nr_class;
}

void svm_get_labels(const svm_model *model, int* label)
{
    if (model->label != NULL)
        for(int i=0;i<model->nr_class;i++)
            label[i] = model->label[i];
}

double svm_get_svr_probability(const svm_model *model)
{
    if ((model->param.svm_type == EPSILON_SVR || model->param.svm_type == NU_SVR) &&
        model->probA!=NULL)
        return model->probA[0];
    else
    {
        LL_CRITICAL("Model doesn't contain information for SVR probability inference\n");
        return 0;
    }
}
