import os
import argparse
from xml.dom import minidom

PATTERNS = [
  ('.ctor', ['cPtr', 'cMemoryOwn']),
  ('swigCMemOwn', []),
  ('SwigGetRawPtr', [])
]

def cleanupXml(filePath, patterns):
  xmlDoc = minidom.parse(filePath) 
  membersList = xmlDoc.getElementsByTagName('Members')
  for i in range(0, len(membersList)):
    removeList = []
    members = membersList[i]
    for member in members.getElementsByTagName('Member'):
      for pattern in patterns:
        if member.attributes['MemberName'].value == pattern[0]:
          paramList = member.getElementsByTagName('Parameter')
          if set([param.attributes['Name'].value for param in paramList]) == set(pattern[1]):
            removeList.append(member)
    for member in removeList:
      members.removeChild(member)
  xmlDoc.writexml(open(filePath, 'w'))

def cleanupDirectory(docDir, patterns):
  for dirName in os.listdir(docDir):
    dirPath = os.path.join(docDir, dirName)
    if not os.path.isdir(dirPath) or dirName.startswith('.'):
      continue
    for fileName in os.listdir(dirPath):
      if not fileName.lower().endswith('.xml'):
        continue
      filePath = os.path.join(dirPath, fileName)
      cleanupXml(filePath, patterns)

parser = argparse.ArgumentParser()
parser.add_argument('docDir', help='Document directory')
args = parser.parse_args()

cleanupDirectory(args.docDir, PATTERNS)
