#include "utils.h"
#include "single_cell_version.h"
#include <string.h>
int usage()
{
    fprintf(stderr, "\nSingleCellTools - Collection of tools to process single cell omics data.\n");
    fprintf(stderr, "Version : %s\n", SINGLECELL_VERSION);
    fprintf(stderr, "\nCommands :\n");
    fprintf(stderr, "\n--- Processing FASTQ\n");
    fprintf(stderr, "    parse      Parse cell barcode, sample barcode and UMI from fastq reads to read name.\n");
    fprintf(stderr, "    trim       Trim TN5 mosic ends or polyAs.\n");
    fprintf(stderr, "    fsort      Sort fastq records by tags.\n");
    fprintf(stderr, "    assem      Assem reads per fastq block (specified with -tag, fastq need be sorted)\n");
    fprintf(stderr, "    segment    Check predefined segments from reads. Designed to process LFR reads. *experimental*\n");
    fprintf(stderr, "\n--- Processing BAM\n");
    fprintf(stderr, "    sam2bam    Convert SAM format to BAM, and parse cell barcodes at read name.\n");
    fprintf(stderr, "    rmdup      Remove PCR duplicates, consider cell barcodes and UMI tag.\n");
    fprintf(stderr, "    pick       Pick alignments of specific cells.\n");
    fprintf(stderr, "    anno       Annotate peak or gene names into BAM attributions.\n");
    fprintf(stderr, "    attrcnt    Count raw reads and reads with predefined tag for each cell.\n");
    fprintf(stderr, "    count      Count matrix.\n");
    // fprintf(stderr, "    bam2bw     Convert BAM to BigWig file.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Author : Shi Quan [shiquan(AT)genomics.cn]\n");
    fprintf(stderr, "Homepage : https://github.com/shiquan/SingleCellTools\n");
    return 1;
}
int main(int argc, char *argv[])
{
    extern int fastq_prase_barcodes(int argc, char *argv[]);
    extern int fastq_trim_adaptors(int argc, char *argv[]);

    extern int sam2bam(int argc, char *argv[]);
    extern int bam_rmdup(int argc, char *argv[]);
    extern int bam_anno_attr(int argc, char *argv[]);
    extern int bam_count_attr(int argc, char *argv[]);
    extern int bam_pick(int argc, char *argv[]);
    extern int count_matrix(int argc, char *argv[]);
    extern int fsort(int argc, char ** argv);
    extern int assem(int argc, char **argv);
    extern int check_segment(int argc, char **argv);
    
    if (argc == 1) return usage();
    else if (strcmp(argv[1], "parse") == 0) return fastq_prase_barcodes(argc-1, argv+1);
    else if (strcmp(argv[1], "trim") == 0) return fastq_trim_adaptors(argc-1, argv+1);
    else if (strcmp(argv[1], "fsort") == 0) return fsort(argc-1, argv+1);
    else if (strcmp(argv[1], "sam2bam") == 0) return sam2bam(argc-1, argv+1);
    else if (strcmp(argv[1], "rmdup") == 0) return bam_rmdup(argc-1, argv+1);
    else if (strcmp(argv[1], "anno") == 0) return bam_anno_attr(argc-1, argv+1);
    else if (strcmp(argv[1], "attrcnt") == 0) return bam_count_attr(argc-1, argv+1);
    else if (strcmp(argv[1], "pick") == 0) return bam_pick(argc-1, argv+1);
    else if (strcmp(argv[1], "count") == 0) return count_matrix(argc-1, argv+1);
    else if (strcmp(argv[1], "assem") == 0)  return assem(argc-1, argv+1);
    else if (strcmp(argv[1], "segment") == 0) return check_segment(argc-1, argv+1);

                    
    else return usage();
    return 0;
}