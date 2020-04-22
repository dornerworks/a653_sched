/******************************************************************************
 * a653_sched.c
 *
 * ARLX (ARINC653 Realtime Linux/Xen) Scheduling Command
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2010, DornerWorks, Ltd. <DornerWorks.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <xenctrl.h>
#include <xenstore.h>

#include <uuid/uuid.h>

#ifndef MILLISECS_TO_NANOSECS
#define MILLISECS_TO_NANOSECS(ms) ((ms)*1000000ll)
#endif

#define MIN_TIMESLICE 20ULL

#ifndef MICROSECS_TO_NANOSECS
#define MICROSECS_TO_NANOSECS(ms) ((ms)*1000ll)
#endif

static struct xen_sysctl_arinc653_schedule sched;

static void usage(char *pname, int rc)
{
    printf("Usage: %s <domname:runtime> ...\n\n", pname);
    printf("\tAll times are in milliseconds by default\n");
    printf("\tMajor Frame is the sum of all runtimes\n");
    printf("Options:\n");
    printf("\t--help|-h\t\tdisplay this usage information\n");
    printf("\t--ids|-i\t\tUser provided UUIDs\n");
    printf("\t--names|-n\t\tUser provided UUIDs as ASCII\n");
    printf("\t--micro|-i\t\tTimes are in microseconds\n");
    printf("\t--pool|-p\t\tpool name\n");
    exit(rc);
}

int arinc653_schedule_set(uint32_t cpupool_id,
                          void *as)
{
    xc_interface *xci = xc_interface_open(NULL, NULL, 0);

    if (xci == NULL)
    {
        fprintf(stderr, "Could not open xenctrl interface!\n");
        return -1;
    }

    int rc = xc_sched_arinc653_schedule_set(xci, 
                                            cpupool_id,
                                            (struct xen_sysctl_arinc653_schedule *)as);
    if (xc_interface_close(xci))
    {
        fprintf(stderr, "xc_interface_close() returned nonzero!\n");
    }

    return rc;
}

int find_domain_uuid(struct xs_handle *xsh, char *name,
                     void* uuid)
{
    int i;

    char path[64];
    void* pxs;
    char** pdir;
    unsigned int num;
    unsigned int len;

    strcpy(path, "/vm");
    pdir = xs_directory(xsh, XBT_NULL, path, &num);

    if (NULL == pdir)
    {
        fprintf(stderr, "Could not open %s xenstore directory!\n",
                path);
        return -1;
    }

    for(i = 0; i < num; ++i)
    {
        sprintf(path, "/vm/%s/name", pdir[i]);
        pxs = xs_read(xsh, XBT_NULL, path, &len);

        if (NULL == pxs)
        {
            fprintf(stderr, "Could not read \"%s\" from xenstore!\n",
                    path);
            free(pdir);
            return -1;
        }

        if (0 == strcmp(pxs, name))
        {
            if (uuid_parse(pdir[i], uuid) < 0)
            {
               fprintf(stderr, "\"%s\" must not be a uuid.\n",
                       pdir[i]);

            }
            free(pxs);
            free(pdir);
            return 0;
        }

        free(pxs);
    }

    free(pdir);
    return -1;
}

int find_pool_id(struct xs_handle *xsh, char *name,
                 int *pid)
{
    int i;

    char path[64];
    void* pxs;
    char** pdir;
    unsigned int num;
    unsigned int len;

    strcpy(path, "/local/pool");
    pdir = xs_directory(xsh, XBT_NULL, path, &num);

    if (NULL == pdir)
    {
        fprintf(stderr, "Could not open %s xenstore directory!\n",
                path);
        return -1;
    }

    for(i = 0; i < num; ++i)
    {
        sprintf(path, "/local/pool/%s/name", pdir[i]);
        pxs = xs_read(xsh, XBT_NULL, path, &len);

        if (NULL == pxs)
        {
            fprintf(stderr, "Could not read \"%s\" from xenstore!\n",
                    path);
            free(pdir);
            return -1;
        }

        if (0 == strcmp(pxs, name))
        {
            *pid = atoi(pdir[i]);
            free(pxs);
            free(pdir);
            return 0;
        }

        free(pxs);
    }

    free(pdir);
    return -1;
}

int main(int argc, char *argv[])
{
    int names = 0;
    int uuids = 0;
    int micro = 0;

    int opt, longindex;
    static const struct option longopts[] = {
        {"names",       no_argument,        &names, 'n'},
        {"ids",         no_argument,        &uuids, 'i'},
        {"micro",       no_argument,        &micro, 'u'},
        {"help",        no_argument,        NULL,   'h'},
        {"pool",        required_argument,  NULL,   'p'},
        {NULL,          0,                  NULL,   0}
    };

    int i;
    int j;
    char *arg_str;
    char *last_str;
    int64_t maj_time = 0;

    uint64_t runtime = 0;

    uint32_t pool = 0;

    struct xs_handle *xsh;

    xsh = xs_open(XS_OPEN_READONLY);
    if (NULL == xsh)
    {
        fprintf(stderr, "Failed to get xenstore handle.\n");
        return 1;
    }

    while ((opt = getopt_long(argc, argv, "hinp:", longopts, &longindex)) != -1)
    {
        switch (opt)
        {
            case 'h':
                usage(argv[0], 0);
                break;
            case 'n':
                printf("Using command-line provided names instead of XenStore.\n");
            case 'i':
                printf("Using command-line provided UUIDs instead of XenStore.\n");
                break;
            case 'u':
                printf("Micro-second timeslices enabled.\n");
                break;
            case 'p':
                if (NULL == optarg)
                {
                    usage(argv[0], 0);
                }

                if (find_pool_id(xsh, optarg, &pool) < 0)
                {
                   printf("Couldn't find pool named \"%s\".\n", optarg);
                   printf("Using default pool.\n", last_str);
                }
                break;
            default:
                usage(argv[0], 2);
                break;
        }
    }

    if ((optind >= argc) || (argc < 2))
    {
        usage(argv[0], 2);
    }
    else if ((argc - 1) >= ARINC653_MAX_DOMAINS_PER_SCHEDULE)
    {
      printf("Only %d domains are supported.  Given %d.\n",
             ARINC653_MAX_DOMAINS_PER_SCHEDULE,
             (argc - 1));

      usage(argv[0], 2);
    }

    j = 0;
    for (i = optind; (i < argc); ++i)
    {

        last_str = argv[i];
        arg_str = strchr(last_str, ':');

        if (arg_str != NULL)
        {
            *arg_str = '\0';

            if (strlen(last_str) != 0)
            {

                if (uuids)
                {
                   uuid_parse(last_str, sched.sched_entries[i].dom_handle);
                }
                else if (names)
                {
                   /* UUID is Domain name in ASCII */
                   strncpy((char *)sched.sched_entries[j].dom_handle,
                           last_str,
                           sizeof(sched.sched_entries[j].dom_handle));
                }
                else if (find_domain_uuid(xsh, last_str,
                                     sched.sched_entries[j].dom_handle) < 0)
                {
                   printf("Couldn't find VM named \"%s\".\n", last_str);
                   printf("Skipping it.\n", last_str);
                   continue;
                }
            }
            else
            {
               /* Dom0 entry */
               memset(sched.sched_entries[j].dom_handle, 0,
                      sizeof(sched.sched_entries[j].dom_handle));
            }

            sched.sched_entries[j].vcpu_id = 0;
        }
        else
        {
            printf("Invalid argument (%s).\n", argv[i]);
            usage(argv[0], 2);
        }

        ++arg_str;

        runtime = atoi(arg_str);

        if (runtime <= 0)
        {
           printf("Invalid run time (%llu).\n", runtime);
           usage(argv[0], 2);
        }

        if (micro)
        {
           if (runtime <= MIN_TIMESLICE)
           {
              printf("\t***Warning: Timeslices of %d us or less are likely"
                     " to starve the domain.***\n", MIN_TIMESLICE);
           }

           sched.sched_entries[j].runtime = MICROSECS_TO_NANOSECS(runtime);
        }
        else
        {
           sched.sched_entries[j].runtime = MILLISECS_TO_NANOSECS(runtime);
        }


        maj_time += sched.sched_entries[j].runtime;
        ++j;
    }

    sched.major_frame = maj_time;
    sched.num_sched_entries = j;

    if (0 == j)
    {
       printf("No valid domain entries.\n");
       usage(argv[0], 2);
    }
    else if (arinc653_schedule_set(pool, (void*)&sched) < 0)
    {
        printf("operation failed\n");
        return 1;
    }
    else
    {
        printf("operation succeeded\n");
        return 0;
    }

    xs_close(xsh);
}
