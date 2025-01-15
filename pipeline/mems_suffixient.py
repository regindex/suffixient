#!/usr/bin/env python3

import sys, time, argparse, subprocess, os.path, threading

Description = """
Tool to build the suffixient of a text using PFP.
The input file cannot contain the characters 0, 1 or 2 which are
used internally by PFP construction algorithm.
"""


dirname         = os.path.dirname(os.path.abspath(__file__))

bigbwt_dirname  = os.path.join(dirname, "_deps/bigbwt-build")
tools_dirname   = os.path.join(dirname, "sources")
index_dirname   = os.path.join(dirname, "index")

parse_exe       = os.path.join(bigbwt_dirname, "pscan.x")
parsebwt_exe    = os.path.join(bigbwt_dirname, "bwtparse")
parsebwt_exe64  = os.path.join(bigbwt_dirname, "bwtparse64")
pfbwt_exe       = os.path.join(bigbwt_dirname, "pfbwt.x")
pfbwtNT_exe     = os.path.join(bigbwt_dirname, "pfbwtNT.x")
pfbwt_exe64     = os.path.join(bigbwt_dirname, "pfbwt64.x")
pfbwtNT_exe64   = os.path.join(bigbwt_dirname, "pfbwtNT64.x")
suffixient_exe  = os.path.join(tools_dirname,  "pfp_suffixient")
one_pass_exe    = os.path.join(tools_dirname,  "one-pass-lcs")
test_index_exe  = os.path.join(index_dirname,  "build_test_index")

def main():
  parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawTextHelpFormatter)
  #parser.add_argument('input', help='input file name', type=str)
  parser.add_argument('--input', help='input file path', default="None", type=str)
  parser.add_argument('--mems', help='patterns file path', default="None", type=str)
  parser.add_argument('-a', '--algorithm', help='one-pass|linear|PFP (def. one-pass)', default="one-pass", type=str)
  parser.add_argument('-w', '--wsize', help='sliding window size (def. 10)', default=10, type=int)
  parser.add_argument('-p', '--mod', help='hash modulus (def. 100)', default=100, type=int)
  parser.add_argument('-t', help='number of helper threads (def. None)', default=0, type=int)
  #parser.add_argument('-o', help='output file path (def. None)', default="", type=str)
  #parser.add_argument('-v',  help='verbose',action='store_true')
  parser.add_argument('-c',  help='print size of the suffixient set to console',action='store_true')
  parser.add_argument('-r',  help='print the number of runs of the BWT',action='store_true')
  parser.add_argument('-i',  help='invert the text before running PFP',action='store_true')
  #parser.add_argument('-m', help='print memory usage',action='store_true')
  args = parser.parse_args()

  if args.algorithm == "PFP":
    logfile_name = args.input + ".suffixient.log"
    # get main directory
    args.bigbwt_dir = os.path.split(sys.argv[0])[0]
    print("Sending logging messages to file:", logfile_name)
    # invert the input file
    if args.i:
      text = ""
      with open(args.input,"r") as file:
        text = file.read()
      text = reversed(text)
      args.input += ".inv"
      with open(args.input,"a") as file:
        file.write("".join(text))
    with open(logfile_name,"a") as logfile:
      args.logfile = logfile
      args.logfile_name = logfile_name
      # ---------- parsing of the input file
      start0 = start = time.time()
      if args.t>0:
        command = "{exe} {file} -w {wsize} -p {modulus} -t {th}".format(
                exe = os.path.join(args.bigbwt_dir,parse_exe),
                wsize=args.wsize, modulus = args.mod, th=args.t, file=args.input)
      else:
        command = "{exe} {file} -w {wsize} -p {modulus}".format(
                exe = os.path.join(args.bigbwt_dir,parse_exe),
                wsize=args.wsize, modulus = args.mod, file=args.input)
      #if args.v: command += " -v"
      #if args.f: command += " -f"
      command += " -s"
      print("==== Parsing. Command:", command)
      if(execute_command(command,logfile,logfile_name)!=True):
        return
      print("Elapsed time: {0:.4f}".format(time.time()-start))

      print("Total construction time: {0:.4f}".format(time.time()-start0))
      # ---- print elapsed time to file
      command = "echo Total construction time: {0:.4f}".format(time.time()-start0)
      if(execute_command(command,logfile,logfile_name)!=True):
        return

      # ---- delete intermediate files
      delete_temp_files(args,logfile,logfile_name)

      print("Elapsed time: {0:.4f}".format(time.time()-start))

    print("==== Done")

    # ---- run suffixient construction
    command = "{exe} -i {file} -w {wsize} -n {size}".format(
            exe = os.path.join(args.bigbwt_dir,suffixient_exe),
            file = args.input, wsize = args.wsize, size =  os.path.getsize(args.input)+1)
    if args.o != "":
      command += " -o {out_file}".format(out_file=args.o)
    if args.c:
      command += " -p"
    if args.r:
      command += " -r"
    print("==== Compute suffixient. Command:", command)
    #if(execute_command(command,logfile,logfile_name)!=True):
    #  return
    subprocess.run(command.split())

  else:
    logfile_name = args.input + ".suffixient.log"
    with open(logfile_name,"a") as logfile:
      start0 = start = time.time()
      command = "{exe} -o {output}".format(
              exe = os.path.join(dirname,"sources/one-pass-lcs"),
              output=args.input+".suffixient")
      
      print("==== Suffixient set construction. Command:", command)
      if(execute_command_stdIn(command,args.input,logfile,logfile_name)!=True):
        return
      print("Elapsed time: {0:.4f}".format(time.time()-start))
      
      start = time.time()
      command = "{exe} {input} {suff_set} {suff_lcs} {suff_first} {patterns}".format(
                exe=test_index_exe, input=args.input, suff_set=args.input+".suffixient",
                suff_lcs=args.input+".suffixient.lcs", suff_first=args.input+".suffixient.first",
                patterns=args.mems)
      print("==== Suffixient index construction and MEMs testing. Command:", command)
      if(execute_command(command,logfile,logfile_name)!=True):
        return
      print("Elapsed time: {0:.4f}".format(time.time()-start))

      command = "echo Total Elapsed time: {0:.4f}".format(time.time()-start0)
      if(execute_command(command,logfile,logfile_name)!=True):
        return


# delete intermediate files
def delete_temp_files(args,logfile,logfile_name):
    #if args.k==False:
    if True:
      print("==== Deleting temporary files.") # no need to show the command
      command = "rm -f {file}.parse_old {file}.last {file}.bwlast {file}.ilist".format(file=args.input)
      #command = "rm -f {file}.parse {file}.parse_old {file}.last {file}.bwlast {file}.dict {file}.ilist {file}.occ".format(file=args.input)
      if(execute_command(command,logfile,logfile_name)!=True):
        return
      for i in range(args.t):
        command = "rm -f {file}.{i}.parse_old {file}.{i}.last".format(file=args.input, i=i)
        if(execute_command(command,logfile,logfile_name)!=True):
          return
      
      command = "rm -f {file}.sai {file}.bwsai".format(file=args.input);
      if(execute_command(command,logfile,logfile_name)!=True):
        return
      for i in range(args.t):
        command = "rm -f {file}.{i}.sai".format(file=args.input, i=i)
        if(execute_command(command,logfile,logfile_name)!=True):
          return

# compute hash digest for a file
def file_digest(name,logfile):
    try:
      hash_command = "{exe} {infile}".format(exe=shasum_exe, infile=name)
      hashsum = subprocess.check_output(hash_command.split(),stderr=logfile)
      hashsum = hashsum.decode("utf-8").split()[0]
    except:
      hashsum = "Error!"
    return hashsum

# execute command: return True is everything OK, False otherwise
def execute_command(command,logfile,logfile_name,env=None):
  try:
    #subprocess.run(command.split(),stdout=logfile,stderr=logfile,check=True,env=env)
    subprocess.check_call(command.split(),stdout=logfile,stderr=logfile,env=env)
  except subprocess.CalledProcessError:
    print("Error executing command line:")
    print("\t"+ command)
    print("Check log file: " + logfile_name)
    return False
  return True

# execute command: return True is everything OK, False otherwise
def execute_command_stdIn(command,inputfile,logfile,logfile_name,env=None):
  try:
    with open(inputfile,'rb') as a:
      subprocess.Popen(command.split(),stdin=a,stderr=logfile,env=env)
  except subprocess.CalledProcessError:
    print("Error executing command line:")
    print("\t"+ command)
    print("Check log file: " + logfile_name)
    return False
  return True

##########################
if __name__ == '__main__':
    main()