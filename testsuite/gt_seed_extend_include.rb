def build_encseq(indexname, sequencefile)
  return "#{$bin}gt encseq encode -des no -sds no -md5 no " +
    "-indexname " + indexname + " " + sequencefile
end

def split_output(key)
  run "grep '^[0-9]' #{key}.out"
  run "cut -f 1-3,5,8-10 -d ' ' #{last_stdout}"
  run "sort #{last_stdout}"
  run "mv #{last_stdout} #{key}.coords"
  grep "#{key}.coords", /^\d+ \d+ \d+ \d+ \d+ \d+ \d+\.\d+$/
end

seeds = [170039800390891361279027638963673934519,
         189224055964190192145211745471700259490,
         80497492730600996116307599313171942911,
         287388662527785534859766605927664912964,
         296993902622042895065571446462399141014,
         267703755545645415708106570926631501781,
         31312989670081360011048989184888532950,
         54623490901073137545509422160541861122,
         255642063275935424280602245704332672807]

# KmerPos and SeedPair verification
Name "gt seed_extend: small_poly, no extension, verify lists"
Keywords "gt_seed_extend only-seeds verify debug-kmer debug-seedpair small_poly"
Test do
  run_test build_encseq("small_poly", "#{$testdata}small_poly.fas")
  run_test "#{$bin}gt seed_extend -only-seeds -verify -seedlength 10 " +
           "-debug-kmer -debug-seedpair -ii small_poly"
  run "gunzip -c #{$testdata}seedextend1.out.gz | cmp -s #{last_stdout}"
  run_test "#{$bin}gt seed_extend -only-seeds -verify " +
           "-debug-kmer -debug-seedpair -ii small_poly | wc -l | grep 792"
  run_test "#{$bin}gt seed_extend -only-seeds -verify -seedlength 13 " +
           "-debug-seedpair -ii small_poly"
  grep last_stdout, /\# SeedPair \(0,2,12,12\)/
  grep last_stdout, /\# SeedPair \(0,2,13,12\)/
end

# Compare xdrop and greedy extension
Name "gt seed_extend: small_poly, xdrop vs greedy extension"
Keywords "gt_seed_extend extendgreedy extendxdrop small_poly"
Test do
  run_test build_encseq("small_poly", "#{$testdata}small_poly.fas")
  run_test "#{$bin}gt seed_extend -extendxdrop 97 " +
           "-l 10 -ii small_poly"
  run "cmp -s #{last_stdout} #{$testdata}seedextend3.out"
  run_test "#{$bin}gt seed_extend -extendgreedy 97 " +
           "-l 10 -ii small_poly"
  run "cmp -s #{last_stdout} #{$testdata}seedextend3.out"
end

# Memlimit and maxfreq options (filter)
Name "gt seed_extend: at1MB, no extension, memlimit, maxfreq"
Keywords "gt_seed_extend at1MB memlimit maxfreq"
Test do
  run_test build_encseq("at1MB", "#{$testdata}at1MB")
  run_test "#{$bin}gt seed_extend -verify -debug-seedpair -memlimit 10MB " +
           "-ii at1MB -only-seeds -no-reverse -seedlength 14"
  grep last_stderr, /Only 14-mers occurring <= 3 times will be considered, /
    /due to small memlimit. Expect 50496 seed pairs./
  run "gunzip -c #{$testdata}seedextend2.out.gz | cmp -s #{last_stdout}"
  run_test "#{$bin}gt seed_extend -only-seeds -v -maxfreq 5 -ii at1MB"
  grep last_stdout, /...found 622939 10-mers/
  grep last_stdout, /...collected 305756 seed pairs/
  grep last_stdout, /...collected 235705 rev.compl. seed pairs/
  run_test "#{$bin}gt seed_extend -only-seeds -v -maxfreq 11 -memlimit 1GB " +
           "-ii at1MB"
  grep last_stdout, /Set k-mer maximum frequency to 11, expect 460986 seed/
end

# Filter options
Name "gt seed_extend: diagbandwidth, mincoverage, seedlength"
Keywords "gt_seed_extend filter diagbandwidth mincoverage memlimit"
Test do
  run_test build_encseq("gt_bioseq_succ_3", "#{$testdata}gt_bioseq_succ_3.fas")
  for seedlength in [2, 5, 14, 32] do
    for diagbandwidth in [0, 1, 5, 10] do
      for mincoverage in [1, 10, 50] do
        run_test "#{$bin}gt seed_extend -seedlength #{seedlength} " +
                 "-diagbandwidth #{diagbandwidth} " +
                 "-mincoverage #{mincoverage} " +
                 "-ii gt_bioseq_succ_3", :retval => 0
      end
    end
  end
end

# Extension options
Name "gt seed_extend: greedy sensitivity, l, minidentity"
Keywords "gt_seed_extend extendgreedy sensitivity alignlength history"
Test do
  run_test build_encseq("at1MB", "#{$testdata}at1MB")
  for sensitivity in [90, 97, 100] do
    for alignlength in [2, 80] do
      for minidentity in [70, 80, 99] do
        run_test "#{$bin}gt seed_extend -extendgreedy #{sensitivity} " +
                 "-minidentity #{minidentity} -l #{alignlength} -a " +
                 "-seed-display -ii at1MB", :retval => 0
      end
    end
  end
  run_test "#{$bin}gt seed_extend -extendgreedy -bias-parameters -verify " +
           "-overlappingseeds -a -seed-display -ii at1MB", :retval => 0
end

# Greedy extension options
Name "gt seed_extend: history, percmathistory, maxalilendiff"
Keywords "gt_seed_extend extendgreedy history percmathistory maxalilendiff"
Test do
  run_test build_encseq("at1MB", "#{$testdata}at1MB")
  for history in [10, 50, 64] do
    for percmathistory in [70, 80, 99] do
      for maxalilendiff in [1, 10, 30] do
        run_test "#{$bin}gt seed_extend -maxalilendiff #{maxalilendiff} " +
        "-history #{history} -percmathistory #{percmathistory} -a " +
        "-ii at1MB", :retval => 0
      end
    end
  end
  run_test "#{$bin}gt seed_extend -bias-parameters -overlappingseeds " +
  "-seed-display -ii at1MB", :retval => 0
end

# Xdrop extension options
Name "gt seed_extend: extendxdrop, xdropbelow, cam"
Keywords "gt_seed_extend extendxdrop xdropbelow cam"
Test do
  run_test build_encseq("at1MB", "#{$testdata}at1MB")
  for sensitivity in [90, 100] do
    for xdbelow in [1, 3, 5] do
      for cam in ["encseq", "encseq_reader"] do
        run_test "#{$bin}gt seed_extend -extendxdrop #{sensitivity} " +
                 "-xdropbelow #{xdbelow} -cam #{cam} -ii at1MB", :retval => 0
      end
    end
  end
end

# Invalid arguments
Name "gt seed_extend: failure"
Keywords "gt_seed_extend failure"
Test do
  run_test build_encseq("at1MB", "#{$testdata}at1MB")
  run_test build_encseq("foo", "#{$testdata}foo.fas")
  run_test "#{$bin}gt seed_extend -seedlength 10 -ii foo", :retval => 1
  grep last_stderr, /integer <= 8 \(length of longest sequence\)/
  run_test "#{$bin}gt seed_extend -maxfreq 1 -ii at1MB", :retval => 1
  grep last_stderr, /option "-maxfreq" must be >= 2 to find matching k-mers/
  run_test "#{$bin}gt seed_extend -t 2 -ii at1MB", :retval => 1
  grep last_stderr, /option "-t" must be >= 3 to find matching k-mers/
  run_test "#{$bin}gt seed_extend -memlimit 0MB -ii at1MB", :retval => 1
  grep last_stderr, /argument to option "-memlimit" must be at least 1MB/
  run_test "#{$bin}gt seed_extend -memlimit 1MB -ii at1MB", :retval => 1
  grep last_stderr, /option -memlimit too strict: need at least 21MB/
  run_test "#{$bin}gt seed_extend -memlimit 1KB -ii at1MB", :retval => 1
  grep last_stderr, /integer argument followed by one of the keywords MB and GB/
  run_test "#{$bin}gt seed_extend -extendgreedy -history 65 -benchmark " +
           "-ii at1MB", :retval => 1
  grep last_stderr, /argument to option "-history" must be an integer <= 64/
  run_test "#{$bin}gt seed_extend -percmathistory 140 -extendgreedy -v " +
           "-ii at1MB", :retval => 1
  grep last_stderr, /option "-percmathistory" must be an integer <= 100/
  run_test "#{$bin}gt seed_extend -extendgreedy -cam invalidlongcamstring " +
           "-ii at1MB", :retval => 1
  grep last_stderr, /illegal parameter for option -cam/
  run_test "#{$bin}gt seed_extend -v -ii at1MB at1MB at1MB", :retval => 1
  grep last_stderr, /too many arguments/
  run_test "#{$bin}gt seed_extend -benchmark", :retval => 1
  grep last_stderr, /option "-ii" is mandatory/
  run_test "#{$bin}gt seed_extend -no-reverse -no-forward -ii foo", :retval => 1
  grep last_stderr, /option "-no-reverse" and option "-no-forward" exclude /
                    /each other/
end

# Find synthetic alignments
Name "gt seed_extend: artificial sequences"
Keywords "gt_seed_extend artificial"
Test do
  for seed in seeds do
    for minidentity in [90, 80] do
      run "#{$scriptsdir}gen-randseq.rb --minidentity #{minidentity} " +
      "--seedlength 14 --length 1000 --mode seeded --seed #{seed} " +
      "--seedcoverage 35 --long 10000  --reverse-complement > longseeded.fasta"
      run_test build_encseq("longseeded", "longseeded.fasta")
      run_test "#{$bin}gt seed_extend -extendgreedy -l 900 " +
               "-minidentity #{minidentity} -ii longseeded"
      # Check whether the correct number of alignments are found.
      numalignments = `cat #{last_stdout} | wc -l`.to_i
      # (split db fasta header by '|' and add 1)
      numseeds =  `grep -o '|' <<< $(head -1 longseeded.fasta) | wc -l`.to_i + 1
      if numalignments < numseeds then
        raise TestFailed, "did not find all alignments"
      end
    end
  end
end

# Query sequences
Name "gt seed_extend: self vs query"
Keywords "gt_seed_extend query"
Test do
  seedlength = 14
  extendlength = 100
  minid = 80
  for seed in seeds[0..4] do
    run "#{$scriptsdir}gen-randseq.rb --number 1 --minidentity #{minid} " +
        "--seedlength #{seedlength} --length #{extendlength} --mode seeded " +
        "--namedfiles --seed #{seed}"
    run_test build_encseq("query", "query.fna")
    run_test build_encseq("db", "db.fna")
    run_test build_encseq("all", "db.fna query.fna")
    ["xdrop","greedy"].each do |ext|
      run_test "#{$bin}gt seed_extend -extend#{ext} 100 -l #{extendlength-20} " +
               "-minidentity #{minid} -seedlength #{seedlength} -no-reverse " +
               "-mincoverage #{seedlength} -seed-display -ii all"
      grep last_stdout, /^\d+ \d+ \d+ . \d+ \d+ \d+ \d+ \d+ \d+/
      run "mv #{last_stdout} combined.out"
      split_output("combined")
      run_test "#{$bin}gt seed_extend -extend#{ext} 100 -l #{extendlength-20} " +
               "-minidentity #{minid} -seedlength #{seedlength} -no-reverse " +
               "-mincoverage #{seedlength} -seed-display -ii db -qii query"
      grep last_stdout, /^\d+ \d+ \d+ . \d+ \d+ \d+ \d+ \d+ \d+/
      run "mv #{last_stdout} separated.out"
      split_output("separated")
      run "cmp -s separated.coords combined.coords"
    end
  end
end
