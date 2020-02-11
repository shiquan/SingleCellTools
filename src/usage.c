#include <stdio.h>


int fastq_parse_usage()
{
    fprintf(stderr, "* Parse cell barcode and UMI string from raw FASTQ.\n");
    fprintf(stderr, "parse [options] lane1_1.fq.gz,lane02_1.fq.gz  lane1_2.fq.gz,lane2_2.fq.gz\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -1       [fastq]   Read 1 output.\n");
    fprintf(stderr, " -2       [fastq]   Read 2 output.\n");
    fprintf(stderr, " -config  [json]    Configure file in JSON format. Required.\n");
    fprintf(stderr, " -run     [string]  Run code, used for different library.\n");
    fprintf(stderr, " -cbdis   [file]    Cell barcode sequence and count pairs.\n");
    fprintf(stderr, " -t       [INT]     Thread.\n");
    fprintf(stderr, " -r       [INT]     Records per chunk. [10000]\n");
    fprintf(stderr, " -dis     [file]    Barcode distribution count.\n");
    fprintf(stderr, " -f                 Filter reads based on MGISEQ standard (2 bases < q10 at first 15 bases).\n");
    fprintf(stderr, " -q       [INT]     Drop this read if average sequencing quality below this value.\n");
    fprintf(stderr, " -dropN             Drop the reads if N base in reads or UMI.\n");
    fprintf(stderr, " -report  [csv]     Summary report.\n");
    fprintf(stderr, "\n");
    return 1;
}
int fsort_usage()
{
    fprintf(stderr, "* Sort reads by tags and deduplicate.\n");
    fprintf(stderr, "fastq-sort [options] in.fq\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, " -tag     [TAGS]     Tags, such as CB,UR. Order of these tags is sensitive.\n");
    fprintf(stderr, " -dedup              Remove dna copies with same tags. Only keep reads have the best quality.\n");
    fprintf(stderr, " -dup-tag [TAG]      Tag name of duplication counts. Use with -dedup only. [DU]\n");
    fprintf(stderr, " -list    [file]     White list for first tag, usually for cell barcodes.\n");
    fprintf(stderr, " -@       [INT]      Threads to compress.\n");
    fprintf(stderr, " -o       [fq.gz]    bgzipped output fastq file.\n");
    fprintf(stderr, " -m       [mem]      Memory per thread. [1G]\n");
    fprintf(stderr, " -p                  Input fastq is smart pairing.\n");
    fprintf(stderr, " -T       [prefix]   Write temporary files to PREFIX.nnnn.tmp\n");
    fprintf(stderr, " -report  [csv]      Summapry report.\n");
    fprintf(stderr, "\n");
    return 1;
}

int assemble_usage()
{
    fprintf(stderr, "* Assemble reads from the same barcode.\n");
    fprintf(stderr, "assem [options] in.fq\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -t       [INT]      Threads.\n");
    fprintf(stderr, " -o       [fastq]    Output fastq.\n");
    fprintf(stderr, " -tag     [TAGS]     Tags of read block.\n");
    fprintf(stderr, " -p                  Input fastq is smart paired.\n");
    fprintf(stderr, " -dis     [file]     Assembled length distribution.\n");
    fprintf(stderr, " -report  [csv]      Summary information.\n");
    fprintf(stderr, "\n");
    return 1;
}

int segment_usage()
{
    fprintf(stderr, "* Select defined segments from untigs.\n");
    fprintf(stderr, "Segment [options] in.fq\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, "-config   [json]      Configure file.\n");    
    fprintf(stderr, "-o        [fastq]     Trimed fastq.\n");
    fprintf(stderr, "-sl       [INT]       Seed length for mapping consensus sequence.\n");
    fprintf(stderr, "-t        [INT]       Threads.\n");
    fprintf(stderr, "-tag      [TAGS]      Tags for each read block.\n");
    fprintf(stderr, "-pb       [TAG]       Phase block tag. Default tag is PB.\n");
    fprintf(stderr, "-k                    Keep all reads even no segments detected.\n");
    fprintf(stderr, "-sum      [csv]       Summary report.\n");
    fprintf(stderr, "\n");
    return 1;
}

int sam2bam_usage()
{
    fprintf(stderr, "* Convert SAM to BAM format. Tag information in read name will be extract.\n");
    fprintf(stderr, "sam2bam [options] in.sam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -o       [BAM]       Output file [stdout].\n");
    fprintf(stderr, " -k                   Keep all records in out.bam, include unmapped reads etc.\n");
    fprintf(stderr, " -filter  [BAM]       Filter reads, include unmapped, secondary alignment, mitochondra etc.\n");
    fprintf(stderr, " -q       [INT]       Map quality theshold, mapped reads with smaller mapQ will be filter.\n");
    fprintf(stderr, " -mito    [string]    Mitochondria name. If set ratio of reads in mito will be export in summary file.\n");
    fprintf(stderr, " -maln    [BAM]       Export mitochondria reads into this file instead of standard output file. Use wuth -mito.\n");
    fprintf(stderr, " -r       [INT]       Records per chunk. Default is 1000000.\n");
    fprintf(stderr, " -p                   Input reads are paired.\n");
    fprintf(stderr, " -@       [INT]       Threads to compress bam file.\n");
    fprintf(stderr, " -report  [csv]       Alignment report.\n");
    fprintf(stderr, "\n");
    return 1;    
}

int rmdup_usage()
{
    fprintf(stderr, "* Deduplicate PCR reads based on read position and barcodes.\n");
    fprintf(stderr, "bam_rmdup [options] in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, "   -tag   [TAGS]       Sample tag, cell barcode tag, and/or UMI tag. RG,CB,UR\n");
    fprintf(stderr, "   -t     [INT]        Threads.\n");
    fprintf(stderr, "   -o     [BAM]        Output bam.\n");
    fprintf(stderr, "   -r     [INT]        Records per thread chunk. Default is 10000000.\n");
    fprintf(stderr, "   -k                  Keep duplicates, make flag instead of remove them.\n");
    fprintf(stderr, "\n");
    return 1;
}

int pick_usage()
{
    fprintf(stderr, "* Pick alignment records within barcode list.\n");
    fprintf(stderr, "PickBam [options] in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -list    [file]       Barcode white list.\n");
    fprintf(stderr, " -tag     [TAG]        Barcode tag.\n");
    fprintf(stderr, " -o       [BAM]        Output file.\n");
    fprintf(stderr, " -@       [INT]        Threads to unpack BAM.\n");
    fprintf(stderr, "\n");
    return 1;
}
int anno_usage()
{
    fprintf(stderr, "* Annotate bam records with overlapped function regions. Such as gene, trnascript etc.\n");
    fprintf(stderr, "anno_bam -bed peak.bed -tag PK -o anno.bam in.bam\n");
    fprintf(stderr, "anno_bam -gtf genes.gtf -o anno.bam in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -o        [BAM]       Output bam file.\n");
    fprintf(stderr, " -report   [csv]       Summary report.\n");
    fprintf(stderr, " -@        [INT]       Threads to compress bam file.\n");
    fprintf(stderr, "\nOptions for BED file :\n");
    fprintf(stderr, " -bed      [BED]       Function regions. Three or four columns bed file. Col 4 could be empty or names of this region.\n");
    fprintf(stderr, " -tag      [TAG]       Attribute tag name. Set with -bed.\n");
    fprintf(stderr, "\nOptions for mixed samples.\n");
    fprintf(stderr, " -chr-species  [file]  Chromosome name and related species binding list.\n");
    fprintf(stderr, " -btag     [TAG]       Species tag name. Set with -chr-species.\n");
    fprintf(stderr, "\nOptions for GTF file :\n");
    fprintf(stderr, " -gtf      [GTF]       GTF annotation file. -gtf is conflict with -bed, if set strand will be consider.\n");
    fprintf(stderr, " -tags     [TAGS]      Attribute names. Default is TX,AN,GN,GX,RE.\n");
    fprintf(stderr, " -ignore-strand        Ignore strand of transcript in GTF. Reads mapped to antisense transcripts will also be count.\n");
    fprintf(stderr, " -splice-consider      Reads covered exon-intron edge will also be count.\n");
    fprintf(stderr, " -t        [INT]       Threads to annotate.\n");
    fprintf(stderr, " -chunk    [INT]       Chunk size per thread.\n");
    fprintf(stderr, "\nNotice :\n");
    fprintf(stderr, " * For GTF mode, this program will set tags in default, you could also reset them by -tags.\n");
    fprintf(stderr, "   TX : Transcript id.\n");
    fprintf(stderr, "   AN : Same with TX but set only if read mapped to antisense strand of transcript.\n");
    fprintf(stderr, "   GN : Gene name.\n");
    fprintf(stderr, "   GX : Gene ID.\n");
    fprintf(stderr, "   RE : Region type, should E(exon), N(intron)\n");
    fprintf(stderr, "\n");
    return 1;
}

int bam_corr_usage()
{
    fprintf(stderr, "* Correct error prone barcodes based on frequency.\n");
    fprintf(stderr, "bam_tag_corr [options] in.bam\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, " -o        [BAM]       Output bam.\n");
    fprintf(stderr, " -tag      [TAG]       Tag to correct.\n");
    fprintf(stderr, " -tags-block  [TAGS]   Tags to define each block. Reads in one block will be corrected by frequency.\n");
    fprintf(stderr, " -@        [INT]       Thread to compress BAM file.\n");
    fprintf(stderr, "\n");
    return 1;
}

int bam_attr_usage()
{
    fprintf(stderr, "* Count the frequency of tag values.\n");
    fprintf(stderr, "AttrCount in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -cb       [TAG]      Cell Barcode, or other tag used for each individual.\n");
    fprintf(stderr, " -list     [file]     Cell barcode white list.\n");
    fprintf(stderr, " -tags     [TAGS]     Tags to count.\n");
    fprintf(stderr, " -dedup                Deduplicate the atrributes in each tag.\n");
    fprintf(stderr, " -group    [TAG]      Group tag, count all tags for each group seperately.\n");
    fprintf(stderr, " -o        [file]     Output count table.\n");
    fprintf(stderr, " -q        [INT]      Map Quality to filter bam.\n");
    fprintf(stderr, " -no-header            Ignore header in the output.\n");
    fprintf(stderr, " -@        [INT]      Thread to unpack bam.\n");
    fprintf(stderr, "\n");
    return 1;
}

int bam_extract_usage()
{
    fprintf(stderr, "* Extract tag values from alignments.\n");    
    fprintf(stderr, "bam_extract_tags[options] in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -tags     [TAGS]     Tags to be extracted.\n");
    fprintf(stderr, " -o        [file]     output file. tsv format\n");
    fprintf(stderr, "\n");
    return 1;
}

int bam_count_usage()
{
    fprintf(stderr, "* Count reads or fragments matrix for single-cell datasets.\n");
    fprintf(stderr, "CountMatrix[options] aln.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -tag      [TAG]      Cell barcode tag.\n");
    fprintf(stderr, " -anno-tag [TAG]      Annotation tag, gene or peak.\n");
    fprintf(stderr, " -list     [file]     Barcode white list, used as column names at matrix. If not set, all barcodes will be count.\n");
    fprintf(stderr, " -tab                  Output in three column format. Name\\tCB\\tCount\n");
    fprintf(stderr, " -o        [file]     Output matrix.\n");
    fprintf(stderr, " -umi      [TAG]      UMI tag. Count once if more than one record has same UMI in one gene or peak.\n");
    fprintf(stderr, " -dis-corr             Disable correct UMIs.\n");
    fprintf(stderr, " -q        [INT]      Minimal map quality to filter. Default is 20.\n");
    fprintf(stderr, " -@        [INT]      Threads to unpack BAM.\n");
    fprintf(stderr,"\n");
    return 1;
}

int bam2fq_usage()
{
    fprintf(stderr, "* Convert BAM into fastq.\n");
    fprintf(stderr, "bam2fq -tag CB,UY in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, " -filter               Filter this record if not all tags existed.\n");
    fprintf(stderr, " -fa                   Output fasta instead of fastq.\n");
    fprintf(stderr, " -o        [fastq]    Output file.\n");
    fprintf(stderr, " -@        [INT]      Threads to unpack BAM.\n");
    fprintf(stderr, "\n");
    return 1;
}

int bam_impute_usage()
{
    fprintf(stderr, "* Imputate empty tag by existed tags.\n");
    fprintf(stderr, "bam_impute in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, "  -impute  [TAGS]     Tags to impute.\n");
    fprintf(stderr, "  -block   [TAGS]     Tags to identify each block.\n");
    fprintf(stderr, "  -dist    [INT]      Distance between reads from same block will be imputed.\n");
    fprintf(stderr, "  -k                   Keep unclassified reads in the output.\n");
    fprintf(stderr, "  -@       [INT]      Threads to unpack BAM.\n");
    fprintf(stderr, "\n");
    return 1;
}