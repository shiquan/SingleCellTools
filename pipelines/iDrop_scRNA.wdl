workflow main {
  String root
  String fastq1
  String fastq2
  String outdir
  String refdir
  String STAR
  String sambamba
  String Rscript
  String ID
  String gtf
  String ?runID
  String config
  Int ?expectCell
  Int ?forceCell
  call makedir {
    input:
    Dir=outdir
  }
  call parseFastq {
    input:
    config=config,
    fastq1=fastq1,
    fastq2=fastq2,
    outdir=makedir.Outdir,
    runID=runID,
    root=root,
  }
  call cellCalling {
    input:
    root=root,
    count=parseFastq.count,
    outdir=outdir,
    Rscript=Rscript,
    expectCell=expectCell,
    forceCell=forceCell,  
  }
  call fastq2bam {
    input:
    fastq=parseFastq.fastq,
    outdir=outdir,
    STAR=STAR,
    refdir=refdir,
    root=root
  }
  call sortBam {
    input:
    bam=fastq2bam.bam,
    sambamba=sambamba,
    gtf=gtf,
    root=root,
    list=cellCalling.list,
    outdir=outdir
  }
}

task makedir {
  String Dir
  command {
    echo "[`date +%F` `date +%T`] workflow start" > ${Dir}/workflowtime.log
    mkdir -p ${Dir}
    mkdir -p ${Dir}/outs
    mkdir -p ${Dir}/temp
  }
  output {
    String Outdir="${Dir}"
  }
}
task parseFastq {
  String config
  String fastq1
  String fastq2
  String outdir
  String ?runID
  String root  
  command {
    ${root}/SingleCellTools parse -t 15 -f -q 20 -dropN -config ${config} -cbdis ${outdir}/temp/barcode_counts_raw.txt -run ${default="1" runID} -report ${outdir}/temp/sequencing_report.txt ${fastq1} ${fastq2}  > ${outdir}/temp/reads.fq
  }
  output {
    String count="${outdir}/temp/barcode_counts_raw.txt"
    String fastq="${outdir}/temp/reads.fq"
    String sequencingReport="${outdir}/temp/sequencing_report.txt"
  }
}

task cellCalling {
  String count
  String outdir
  String Rscript
  String root
  Int ?expectCell
  Int ?forceCell    
  command {    
    ${Rscript} ${root}/scripts/scRNA_cell_calling.R -i ${count} -o ${outdir}/outs -e ${default=1000 expectCell} -f ${default=0 forceCell}
  }
  output {
    String list="${outdir}/outs/cell_barcodes.txt"
  }
}
task fastq2bam {
  String fastq  
  String outdir
  String STAR
  String refdir
  String root
  command {
    ${STAR} --outStd SAM --genomeDir ${refdir} --readFilesIn ${fastq} | ${root}/SingleCellTools sam2bam -o ${outdir}/temp/aln.bam -report ${outdir}/temp/alignment_report.txt -maln ${outdir}/temp/mito.bam /dev/stdin
  }
  output {
    String bam="${outdir}/temp/aln.bam"
    String alnReport="{outdir}/temp/alignment_report.txt"
  }
}
task sortBam {
  String bam
  String sambamba
  String root
  String outdir
  String gtf
  String list

  command {
    ${sambamba} sort -t 20 -o ${outdir}/temp/sorted.bam ${outdir}/temp/aln.bam
    ${root}/SingleCellTools anno -gtf ${gtf} -o ${outdir}/temp/anno.bam ${outdir}/temp/sorted.bam
    ${root}/SingleCellTools count -tag CB -anno_tag GN -umi UY -o ${outdir}/outs/count.mtx -list ${list} ${outdir}/temp/anno.bam
    echo "[`date +%F` `date +%T`] workflow end" >> ${outdir}/workflowtime.log
  }
  output {
    String matrix="${outdir}/outs/count.mtx"
  }
}


