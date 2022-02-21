import os
import re
import sys
import subprocess
import argparse
import shutil
import json

SDK_VERSION = '4.4.5'

def makedirs(dir):
  try:
    if not os.path.exists(dir):
      os.makedirs(dir)
  except:
    e = sys.exc_info()[1]
    print("Exception %s while creating directory %s" % (str(e), dir))
    return False
  return True

def copyfile(source, target):
  try:
    shutil.copyfile(source, target)
  except:
    e = sys.exc_info()[1]
    print("Exception %s while copying %s -> %s" % (str(e), source, target))
    return False
  return True

def checksumSHA256(filename):
  import hashlib
  sha256_hash = hashlib.sha256()
  with open(filename, 'rb') as f:
    for block in iter(lambda: f.read(4096), b''):
      sha256_hash.update(block)
  return sha256_hash.hexdigest() 

def makesymlink(dir, source, target):
  currentDir = os.getcwd()
  os.chdir(dir)
  try:
    if os.path.exists(target):
      os.remove(target)
    os.symlink(source, target)
  except:
    e = sys.exc_info()[1]
    print("Exception %s while creating symlink %s -> %s" % (str(e), source, target))
    os.chdir(currentDir)
    return False
  os.chdir(currentDir)
  return True

def execute(cmd, dir, *cmdArgs):
  currentDir = os.getcwd()
  os.chdir(dir)
  cmdLine = [cmd] + list(cmdArgs)
  try:
    code = subprocess.call(cmdLine)
  except:
    e = sys.exc_info()[1]
    print("Exception %s while executing %s (path %s):\n%s" % (str(e), cmd, dir, " ".join(cmdLine)))
    os.chdir(currentDir)
    return False
  os.chdir(currentDir)
  if code != 0:
    print("Error while executing %s (path %s):\n%s" % (cmd, dir, " ".join(cmdLine)))
    return False
  return True

def checkExecutable(cmd, *cmdArgs):
  cmdLine = [cmd] + list(cmdArgs)
  try:
    output = subprocess.check_output(cmdLine, stderr=subprocess.STDOUT)
  except:
    return False
  return True

def cmake(args, dir, cmdArgs):
  return execute(args.cmake, dir, *cmdArgs)

def getBaseDir():
  baseDir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
  if os.name == 'nt':
    baseDir = baseDir.replace("\\", "/")
  return baseDir

def getBuildDir(target, arch=None):
  if arch is None:
    buildDir = '%s/build/%s' % (getBaseDir(), target)
  else:
    buildDir = '%s/build/%s-%s' % (getBaseDir(), target, arch)
  makedirs(buildDir)
  return buildDir

def getDistDir(target):
  distDir = '%s/dist/%s' % (getBaseDir(), target)
  makedirs(distDir)
  return distDir

def getDefaultProfileId():
  return 'standard'

def getProfile(profileIds):
  defines = set()
  cmakeOptions = set()
  allProfileIds = profileIds.split('+')
  if 'lite' not in allProfileIds:
    allProfileIds.append(getDefaultProfileId())
  for profileId in allProfileIds:
    profile = getProfiles()[profileId]
    defines.update(profile.get('defines', '').split(';'))
    cmakeOptions.update(profile.get('cmake-options', '').split(';'))
  return { 'defines': ';'.join(list(defines)), 'cmake-options': ';'.join(list(cmakeOptions)) }

def getProfiles():
  profilesFilename = '%s/sdk_profiles.json' % os.path.dirname(os.path.realpath(__file__))
  with open(profilesFilename, 'r') as f:
    profiles = json.loads(f.read())
    return { str(key): val for key, val in profiles.items() if key != 'free' }

def validProfile(profileIds):
  validProfileIds = getProfiles().keys()
  for profileId in profileIds.split('+'):
    if not profileId in validProfileIds:
      raise argparse.ArgumentTypeError('Profile must be one of or a combination of %s' % ', '.join(validProfileIds))
  return profileIds

def getVersion(buildnumber):
  try:
    lastCommit = "None"
    gitLog = subprocess.Popen(["git", "describe"], stdout=subprocess.PIPE).communicate()[0]
    lastCommit = gitLog.strip()

    branch = "None"
    gitBranches = subprocess.Popen(["git", "branch"], stdout=subprocess.PIPE).communicate()[0]
    for line in gitBranches.split("\n"):
      match = re.match("\\*\s+(.*)", line)
      if match:
        branch = match.group(1)

    return "%s|%s|%s" % (buildnumber, branch, lastCommit)
  except:
    return "%s|%s|%s" % (buildnumber, "UNKNOWN", "UNKNOWN")

def readLines(fileName):
  with open(fileName, 'r') as f:
    lines_in = f.readlines()
  return lines_in

def readUncommentedLines(fileName):
  with open(fileName, 'r') as f:
    lines_in = f.readlines()
  comment = False
  lines_out = []
  for line in lines_in:
    line_prefix = ""
    while True:
      if comment:
        match = re.search(r'[*][/](.*)$', line)
        if not match:
          line = ""
          break
        line = match.group(1)
        comment = False
      else:
        match = re.search(r'(([^/]|[/][^*])*)[/][*](.*)$', line)
        if not match:
          break
        line_prefix = line_prefix + match.group(1)
        line = match.group(3)
        comment = True
    line = line_prefix + line
    if line != "":
      match = re.search(r'^(.*)[/][/].*$', line)
      if match:
        line = match.group(1)
      lines_out.append(line)
  return lines_out

def readLicense():
  with open('%s/LICENSE' % getBaseDir(), 'r') as f:
    licenseText = f.read()
  return licenseText

def applyTemplate(template, valueMap):
  result = template
  for key, value in valueMap.items():
    result = result.replace("$%s$" % key, value)
  return result.split("\n")
