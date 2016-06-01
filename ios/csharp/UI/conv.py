import os
import re
import sys
import argparse
import subprocess
import shutil


def processFile(fileName):
  with open(fileName, 'r') as f:
    lines_in = [line.rstrip('\n') for line in f.readlines()]

  lines_out = []
  comment = False
  summary = False
  for line in lines_in:
    if not comment:
      match = re.search('(.*)[/][*][*]', line)
      if match:
        lines_out.append(match.group(1) + "/// <summary>")
        comment = True
        summary = True
        continue
    else:
      match = re.search('(.*)[*][/]', line)
      if match:
        if summary:
          lines_out.append(match.group(1) + "/// </summary>")
          summary = False
        comment = False
        continue
    if comment:
      line = line.replace('<br>', '')
      match = re.search('(.*) [*](.*)', line)
      if not match:
        continue
      comment = match.group(2)
      match2 = re.search('(@param|@return) (.*)', comment)
      if match2:
        if summary:
          lines_out.append(match.group(1) + "/// </summary>")
          summary = False
        if match2.group(1) == '@param':
          param = match2.group(2)
          match3 = re.search('([^ ]*)\s*(.*)', param)
          lines_out.append(match.group(1) + ('/// <param name="%s">%s</param>' % (match3.group(1), match3.group(2))))
        else:
          lines_out.append(match.group(1) + ('/// <returns>%s</returns>' % match2.group(2)))
      if summary:
        lines_out.append(match.group(1) + ('///%s' % comment))
      continue

    lines_out.append(line)

  # Write processed module
  with open(fileName + '.tmp', 'w') as f:
    f.writelines([line + '\n' for line in lines_out])

parser = argparse.ArgumentParser()
parser.add_argument('fileName')

args = parser.parse_args()
processFile(args.fileName)
