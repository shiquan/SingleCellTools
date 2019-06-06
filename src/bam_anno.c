// annotate Gene or Peak to SAM attribution
#include "utils.h"
#include "number.h"
#include "thread_pool.h"
#include "htslib/khash.h"
#include "htslib/kstring.h"
#include "htslib/sam.h"
#include "htslib/khash_str2int.h"
#include "htslib/kseq.h"
#include "bed_lite.h"
#include "gtf.h"
#include <zlib.h>

static int usage()
{
    fprintf(stderr, "* Annotate bam records with overlapped function regions. Such as gene, trnascript etc.\n");
    fprintf(stderr, "anno_bam -bed peak.bed -tag PK -o anno.bam in.bam\n");
    fprintf(stderr, "anno_bam -gtf genes.gtf -o anno.bam in.bam\n");
    fprintf(stderr, "\nOptions :\n");
    fprintf(stderr, "  -o               Output bam file.\n");
    fprintf(stderr, "  -q               Mapping quality threshold. [255]\n");
    fprintf(stderr, "\nOptions for BED file :\n");
    fprintf(stderr, "  -bed             Function regions. Three or four columns bed file. Col 4 could be empty or names of this region.\n");
    fprintf(stderr, "  -tag             Attribute tag name. Set with -bed\n");
    fprintf(stderr, "\nOptions for GTF file :\n");
    fprintf(stderr, "  -gtf             GTF annotation file. -gtf is conflict with -bed, if set strand will be consider.\n");
    fprintf(stderr, "  -tags            Attribute names. Default is TX,AN,GN,GX,RE.\n");
    fprintf(stderr, "  -ignore-strand   Ignore strand of transcript in GTF. Reads mapped to antisense transcripts will also be count.\n");
    fprintf(stderr, "\nNotice :\n");
    fprintf(stderr, " * For GTF mode, this program will set tags in default, you could also reset them by -tags.\n");
    fprintf(stderr, "   TX : Transcript id.\n");
    fprintf(stderr, "   AN : Same with TX but set only if read mapped to antisense strand of transcript.\n");
    fprintf(stderr, "   GN : Gene name.\n");
    fprintf(stderr, "   GX : Gene ID.\n");
    fprintf(stderr, "   RE : Region type, should E(exon), N(intron)\n");
    return 1;
}
static struct args {
    const char *input_fname;
    const char *output_fname;
    const char *bed_fname;
    const char *tag; // attribute in BAM
    const char *gtf_fname;
    const char *report_fname;
    
    int ignore_strand;
    
    htsFile *fp;
    htsFile *out;
    bam_hdr_t *hdr;
    FILE *fp_report;
    struct gtf_spec *G;

    uint64_t reads_input;
    uint64_t reads_pass_qc;
    uint64_t reads_in_peak;
    // gtf
    uint64_t reads_in_gene;
    uint64_t reads_in_exon;
    uint64_t reads_in_intron;
    uint64_t reads_antisense;

    int qual_thres;
    // todo: make bedaux more smart
    struct bedaux *B;
    struct bed_chr *last;
    int i_bed;
} args = {
    .input_fname     = NULL,
    .output_fname    = NULL,
    .bed_fname       = NULL,    
    .tag             = NULL,    
    .gtf_fname       = NULL,
    .report_fname    = NULL,
    .ignore_strand   = 0,
    .fp              = NULL,
    .out             = NULL,
    .hdr             = NULL,
    .fp_report       = NULL,
    .G               = NULL,    
    .B               = NULL,    
    
    .last            = NULL,
    .i_bed           = -1,

    .reads_input     = 0,
    .reads_pass_qc   = 0,
    .reads_in_peak   = 0,
    .reads_in_gene   = 0,
    .reads_in_exon   = 0,
    .reads_in_intron = 0,
    .reads_antisense = 0,

    .qual_thres      = 255,
};

static char TX_tag[2] = "TX";
static char AN_tag[2] = "AN";
static char GN_tag[2] = "GN";
static char GX_tag[2] = "GX";
static char RE_tag[2] = "RE";

static int parse_args(int argc, char **argv)
{
    int i;
    const char *tags = NULL;
    const char *qual = NULL;
    for (i = 1; i < argc; ) {
        const char *a = argv[i++];
        const char **var = 0;
        if (strcmp(a, "-bed") == 0) var = &args.bed_fname;
        else if (strcmp(a, "-o") == 0 ) var = &args.output_fname;
        else if (strcmp(a, "-report") == 0) var = &args.report_fname;
        else if (strcmp(a, "-tag") == 0) var = &args.tag;
        else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) return 1;
        else if (strcmp(a, "-gtf") == 0) var = &args.gtf_fname;
        else if (strcmp(a, "-tags") == 0) var = &tags;
        else if (strcmp(a, "-q") == 0) var = &qual;
        else if (strcmp(a, "-ignore-strand") == 0) {
            args.ignore_strand = 1;
            continue;
        }
        if (var != 0) {
            if (i == argc) error("Miss an argument after %s.", a);
            *var = argv[i++];
            continue;
        }
        if (args.input_fname == NULL) {
            args.input_fname = a;
            continue;
        }
        error("Unknown argument: %s", a);
    }
    // CHECK_EMPTY(args.bed_fname, "-bed must be set.");

    if (args.bed_fname == NULL && args.gtf_fname == NULL) 
        error("-bed or -gtf must be set.");
    if (args.bed_fname && args.gtf_fname)
        error("-bed is conflict with -gtf, you can only choose one mode.");
    if (args.ignore_strand && args.gtf_fname == NULL)
        error("Only set -ignore-strand with -gtf.");
    
    CHECK_EMPTY(args.output_fname, "-o must be set.");
    CHECK_EMPTY(args.input_fname, "Input bam must be set.");

    if (qual) args.qual_thres = str2int((char*)qual);
    if (args.qual_thres < 0) args.qual_thres = 0; // no filter
    if (tags) {
        kstring_t str = {0,0,0};
        kputs(tags, &str);
        if (str.l != 14) error("Bad format of -tags, require five tags and splited by ','.");
        int n;
        int *s = ksplit(&str, ',', &n);
        if (n != 5) error("-tags required five tag names.");
        
        memcpy(TX_tag, str.s+s[0], 2*sizeof(char));
        memcpy(AN_tag, str.s+s[1], 2*sizeof(char));
        memcpy(GN_tag, str.s+s[2], 2*sizeof(char));
        memcpy(GX_tag, str.s+s[3], 2*sizeof(char));
        memcpy(RE_tag, str.s+s[4], 2*sizeof(char));
    }
    
    if (args.bed_fname) {
        CHECK_EMPTY(args.tag, "-tag must be set.");
        args.B = bed_read(args.bed_fname);
        if (args.B == 0 || args.B->n == 0) error("Bed is empty.");
    }
    else {
        LOG_print("Loading GTF.");
        double t_real;
        t_real = realtime();

        args.G = gtf_read(args.gtf_fname);
        LOG_print("Load time : %.3f sec", realtime() - t_real);
        
        if (args.G == NULL) error("GTF is empty.");
    }
    
    args.fp  = hts_open(args.input_fname, "r");
    CHECK_EMPTY(args.fp, "%s : %s.", args.input_fname, strerror(errno));
    htsFormat type = *hts_get_format(args.fp);
    if (type.format != bam && type.format != sam)
        error("Unsupported input format, only support BAM/SAM/CRAM format.");
    args.hdr = sam_hdr_read(args.fp);
    CHECK_EMPTY(args.hdr, "Failed to open header.");
    //int n_bed = 0;
    
    args.out = hts_open(args.output_fname, "bw");
    CHECK_EMPTY(args.out, "%s : %s.", args.output_fname, strerror(errno));

    if (args.report_fname) {
        args.fp_report = fopen(args.report_fname, "w");
        CHECK_EMPTY(args.fp_report, "%s : %s", args.report_fname, strerror(errno));
    }
    else args.fp_report =stderr;
    
    if (sam_hdr_write(args.out, args.hdr)) error("Failed to write SAM header.");
    
    return 0;
}

int check_is_overlapped_bed(bam_hdr_t *hdr, bam1_t *b, struct bedaux *B)
{
    bam1_core_t *c;
    c = &b->core;
    char *name = hdr->target_name[c->tid];
    if (args.last == NULL || strcmp(B->names[args.last->id], name) != 0) {
        int id = bed_select_chrom(B, name);
        if (id == -1) {
            args.last = NULL;
            return 0;
        }

        args.last = &B->c[id];
        args.i_bed = 0;
    }
    if (args.i_bed == -2) { // out range of bed
        return 0;
    }
    
    for (;;) {
        if (args.i_bed == args.last->n) break;
        if (args.last->b[args.i_bed].end < c->pos+1) args.i_bed++; // iter bed
        else break;
    }

    if (args.i_bed == args.last->n) {
        args.i_bed = -2;
        return 0;
    }
    int end = bam_endpos(b);
    if (end < args.last->b[args.i_bed].start) { // read align before region
        return 0;
    }
    
    uint8_t *tag = bam_aux_get(b, args.tag);
    if (tag) {
        warnings("%s already present at line %s:%d, skip", args.tag, hdr->target_name[c->tid], c->pos+1);
        return 1;
    }
    kstring_t str = {0,0,0};// name buffer
    if (args.last->b[args.i_bed].name == NULL) {
        ksprintf(&str, "%s:%d-%d", B->names[args.last->id], args.last->b[args.i_bed].start, args.last->b[args.i_bed].end);
    }
    else kputs(args.last->b[args.i_bed].name, &str);
    bam_aux_append(b, args.tag, 'Z', str.l+1, (uint8_t*)str.s);
    free(str.s);

    args.reads_in_peak++;
    
    return 0;
}

int check_is_overlapped_gtf(bam_hdr_t *h, bam1_t *b, struct gtf_spec *G)
{
    bam1_core_t *c;
    c = &b->core;
    char *name = h->target_name[c->tid];
    int endpos = bam_endpos(b);
    
    struct gtf_lite *g;
    int n = 0;
    // todo: improve the algorithm of overlap
    g = gtf_overlap_gene(G, name, c->pos, endpos, &n, 1);
    if (n==0) return 1;

    args.reads_in_gene++;

    int i, j;    
    kstring_t trans = {0,0,0};
    
    if (args.ignore_strand == 0) {
        if (c->flag & BAM_FREVERSE) {
            if (g->strand == 0) goto antisense;
        }
        else if (g->strand == 1) goto antisense;
    }

    int l;
    // todo: multi genes
    // TX
    for (i = 0; i < g->n_son; ++i) {
        struct gtf_lite *g1 = &g->son[i];
        if (g1->type != feature_transcript) continue;
        if (c->pos > g1->end|| endpos < g1->start) continue; // not in this trans
        // check isoform
        int in_exon = 0;     
        for (j = 0; j < g1->n_son; ++j) {
            struct gtf_lite *g2 = &g1->son[j];
            if (g2->type != feature_exon) continue; // on check exon for converience
            if (c->pos > g1->end) continue;
            if (endpos < g1->start) continue;
            in_exon = 1;
            break;
        }
        if (in_exon) {
            if (trans.l) kputc(';', &trans);
            kputs(G->transcript_id->name[g1->transcript_id], &trans);
        }
    }
    // TX,RE
    if (trans.l) {
        bam_aux_append(b, TX_tag, 'Z', trans.l+1, (uint8_t*)trans.s);
        bam_aux_append(b, RE_tag, 'A', 1, (uint8_t*)"E");

        args.reads_in_exon++;
    }
    else { // not cover any transcript
        bam_aux_append(b, RE_tag, 'A', 1, (uint8_t*)"I");

        args.reads_in_intron++;
    }
    
    // GN_tag
    char *gene = G->gene_name->name[g->gene_name];
    l = strlen(gene);
    bam_aux_append(b, GN_tag, 'Z', l+1, (uint8_t*)gene);

    // GX
    char *gene_id = G->gene_id->name[g->gene_id];
    l = strlen(gene_id);
    bam_aux_append(b, GX_tag, 'Z', l+1, (uint8_t*)gene_id);
        
    return 0;
    

  antisense:
    args.reads_antisense++;
    // AN    
    for (i = 0; i < g->n_son; ++i) {
        struct gtf_lite *g1 = &g->son[i];
        if (g1->type != feature_transcript) continue;
        if (c->pos > g1->end|| endpos < g1->start) continue; // not in this trans

        // check isoform
        int in_exon = 0;     
        for (j = 0; j < g1->n_son; ++j) {
            struct gtf_lite *g2 = &g1->son[j];
            if (g2->type != feature_exon) continue; // on check exon for converience
            if (c->pos > g1->end) continue;
            if (endpos < g1->start) continue;
            in_exon = 1;
            break;
        }
        if (in_exon) {
            if (trans.l) kputc(';', &trans);
            kputs(G->transcript_id->name[g1->transcript_id], &trans);
        }
    }

    if (trans.l) {
        bam_aux_append(b, AN_tag, 'Z', trans.l+1, (uint8_t*)trans.s);
        bam_aux_append(b, RE_tag, 'A', 1, (uint8_t*)"E");
    }
    else { // not cover any transcript
        bam_aux_append(b, RE_tag, 'A', 1, (uint8_t*)"I");
    }

    
    return 0;
}
void write_report()
{
    if (args.B)
        fprintf(args.fp_report, "Reads Mapped Confidently to Peaks : %.1f%%\n", (float)args.reads_in_peak/args.reads_pass_qc*100);
    else {
        fprintf(args.fp_report, "Reads Mapped Confidently to Genome : %.1f%%\n", (float)args.reads_pass_qc/args.reads_input*100);
        fprintf(args.fp_report, "Reads Mapped Confidently to Gene : %.1f%%\n", (float)args.reads_in_gene/args.reads_pass_qc*100);
        fprintf(args.fp_report, "Reads Mapped Confidently to Exonic Regions: %.1f%%\n", (float)args.reads_in_exon/args.reads_pass_qc*100);
        fprintf(args.fp_report, "Reads Mapped Confidently to Intronic Regions : %.1f%%\n", (float)args.reads_in_intron/args.reads_pass_qc*100);
        fprintf(args.fp_report, "Reads Mapped Antisense to Gene : %.1f%%\n", (float)args.reads_antisense/args.reads_pass_qc*100);
    }
}
void memory_release()
{
    bam_hdr_destroy(args.hdr);
    sam_close(args.fp);
    sam_close(args.out);
    if (args.fp_report != stderr) fclose(args.fp_report);
}
int bam_anno_attr(int argc, char *argv[])
{
    double t_real;
    t_real = realtime();

    if (parse_args(argc, argv)) return usage();
    
    bam1_t *b;
    int ret;
    b = bam_init1();
    
    while ((ret = sam_read1(args.fp, args.hdr, b)) >= 0) {
        args.reads_input++;
        // todo: QC?
        if (b->core.qual < args.qual_thres) continue;
        args.reads_pass_qc++;
        if (args.B) 
            check_is_overlapped_bed(args.hdr, b, args.B); 
        else
            check_is_overlapped_gtf(args.hdr, b, args.G);
        
        if (sam_write1(args.out, args.hdr, b) == -1) error("Failed to write SAM.");
    }


    bam_destroy1(b);

    if (ret != -1) warnings("Truncated file?");

    write_report();
    memory_release();    
    LOG_print("Real time: %.3f sec; CPU: %.3f sec", realtime() - t_real, cputime());
    
    return 0;    
}