import argparse
import os
import re

CPP_RULES = [
  (' * Copyright 2014 Nutiteq Llc. All rights reserved.', ' * Copyright (c) 2016 CartoDB. All rights reserved.'),
  (' * to license terms, as given in https://www.nutiteq.com/license/', ' * to license terms, as given in https://cartodb.com/terms/'),
  ('namespace CartoCSSParserImpl', 'namespace cssparserimpl'),
  ('namespace ColorGeneratorImpl', 'namespace colorgenimpl'),
  ('namespace ColorParserImpl', 'namespace colorparserimpl'),
  ('namespace ExpressionBinderImpl', 'namespace exprbinderimpl'),
  ('namespace ExpressionGeneratorImpl', 'namespace exprgenimpl'),
  ('namespace ExpressionParserImpl', 'namespace exprparserimpl'),
  ('namespace TransformGeneratorImpl', 'namespace transgenimpl'),
  ('namespace TransformParserImpl', 'namespace transparserimpl'),
  ('namespace ValueGeneratorImpl', 'namespace valgenimpl'),
  ('namespace ValueParserImpl', 'namespace valparserimpl'),
  ('namespace CartoCSS', 'namespace css'),
  ('namespace MapnikVT', 'namespace mvt'),
  ('namespace VT', 'namespace vt'),
  ('namespace Nuti', 'namespace carto'),
  ('namespace nmlGL', 'namespace nmlgl'),
  ('NutiteqWatermark', 'CartoWatermark'),
  ('NutiteqPackageManager', 'CartoPackageManager'),
  ('NutiteqOffline', 'CartoOffline'),
  ('NutiteqOnline', 'CartoOnline'),
  ('_NUTITEQONLINE', '_CARTOONLINE'),
  ('_NUTITEQOFFLINE', '_CARTOOFFLINE'),
  ('_NUTITEQPACKAGE', '_CARTOPACKAGE'),
  ('nutiteq_watermark_png', 'carto_watermark_png'),
  (' Nutiteq ', ' Carto '),
  ('NUTITEQ_WATERMARK', 'CARTO_WATERMARK'),
  ('NUTITEQ_MAPS_SDK_', 'CARTO_MOBILE_SDK'),
  ('NUTI_HTTP', 'CARTO_HTTP'),
  ('nutiteq-sdk', 'carto-mobile-sdk'),
  ('Nuti::', 'carto::'),
  ('CartoCSSParserImpl::', 'cssparserimpl::'),
  ('ColorGeneratorImpl::', 'colorgenimpl::'),
  ('ColorParserImpl::', 'colorparserimpl::'),
  ('ExpressionBinderImpl::', 'exprbinderimpl::'),
  ('ExpressionGeneratorImpl::', 'exprgenimpl::'),
  ('ExpressionParserImpl::', 'exprparserimpl::'),
  ('TransformGeneratorImpl::', 'transgenimpl::'),
  ('TransformParserImpl::', 'transparserimpl::'),
  ('ValueGeneratorImpl::', 'valgenimpl::'),
  ('ValueParserImpl::', 'valparserimpl::'),
  ('MapnikVT::', 'mvt::'),
  ('CartoCSS::', 'css::'),
  ('VT::', 'vt::'),
  ('nmlGL::', 'nmlgl::'),
  ('_NUTI_CARTOCSS_', '_CARTO_CARTOCSS_'),
  ('_NUTI_MAPNIKVT_', '_CARTO_MAPNIKVT_'),
  ('_NUTI_SDK_', '_CARTO_MOBILE_SDK_'),
  ('_NUTI_', '_CARTO_'),
  ('vectortiles/VT/', 'vt/'),
  ('vectortiles/MapnikVT/', 'mapnikvt/'),
  ('vectortiles/CartoCSS/', 'cartocss/'),
  ('vectorelements/NML/', 'nml/nmlpackage/'),
  ('renderers/nmlGL/', 'nml/'),
  ('VT/', 'vt/'),
  ('MapnikVT/', 'mapnikvt/'),
  ('CartoCSS/', 'cartocss/'),
  ('nmlGL/', 'nml/'),
  ('Routing/', 'routing/')
]

JAVA_RULES = [
  ('Nutiteq Maps SDK', 'Carto Mobile Maps SDK'),
  ('com.nutiteq.', 'com.carto.'),
  ('nutiteq_maps_sdk', 'carto_mobile_sdk'),
  ('_nutiteqsdk_preferences', '_carto_mobile_sdk1_preferences')
]

CSHARP_RULES = [
  ('NutiteqMapsSDK.', 'CartoMobileSDK.'),
  ('Nutiteq Maps SDK', 'Carto Mobile SDK'),
  ('(c) Nutiteq 2015', '(c) CartoDB 2016'),
  ('namespace Nutiteq', 'namespace Carto'),
  ('libnutiteq_maps_sdk', 'libcarto_mobile_sdk'),
  ('Nutiteq.', 'Carto.'),
  ('Nutiteq', 'CartoDB'),
  ('_nutiteqsdk_preferences', '_carto_mobile_sdk1_preferences')
]

SWIG_RULES = [
  ('Nuti::', 'carto::'),
  ('NUTITEQPACKAGEMANAGER', 'CARTOPACKAGEMANAGER'),
  ('NUTITEQOFFLINE', 'CARTOOFFLINE'),
  ('NUTITEQONLINE', 'CARTOONLINE'),
  ('NutiteqPackageManager', 'CartoPackageManager'),
  ('NutiteqOffline', 'CartoOffline'),
  ('NutiteqOnline', 'CartoOnline'),
  ('_NUTI_', '_CARTO_'),
  ('com.nutiteq.', 'com.carto.'),
  ('"NutiSwig.i"', '<cartoswig.i>')
]

RDXML_RULES = [
  ('NutiteqMapsSDK', 'CartoMobileSDK')
]

CMAKE_RULES = [
  ('nutiteq_maps_sdk', 'carto_mobile_sdk'),
  ('_NUTI_SDK_', '_CARTO_MOBILE_SDK_')
]

EXTENSION_RULES = {
  '.h':      CPP_RULES,
  '.cpp':    CPP_RULES,
  '.mm':     CPP_RULES,
  '.java':   JAVA_RULES,
  '.cs':     CSHARP_RULES,
  '.i':      SWIG_RULES,
  '.rd.xml': RDXML_RULES,
  '.txt':    CMAKE_RULES
}

FILENAME_RULES = [
  ('Nuti.h', 'Carto.h'),
  ('NutiNative.h', 'CartoNative.h'),
  ('Nutiteq', 'Carto')
]

def replace_tab(s, tabstop = 4):
  result = str()
  replace = True
  for c in s:
    if c == '\t' and replace:
      result += ' '
      while (len(result) % tabstop != 0):
        result += ' '
    else:
      result += c
      replace = False
  return result

def replace_tabs(content):
  for i in range(0, len(content)):
    content[i] = replace_tab(content[i])
  return content

def replace_rule(content, rule):
  substr, replacement = rule
  for i in range(0, len(content)):
    content[i] = content[i].replace(substr, replacement)
  return content

def replace_rules(filename, content, reverse):
  for ext, rules in EXTENSION_RULES.items():
    if filename.endswith(ext):
      for match1, match2 in rules:
        content = replace_rule(content, (match2, match1) if reverse else (match1, match2))
  return content

def process_filename(path, reverse):
  for rule in FILENAME_RULES:
    prefix, replacement = rule
    if reverse:
      prefix, replacement = replacement, prefix
    base, filename = os.path.split(path)
    if filename.startswith(prefix):
      return os.path.join(base, replacement + filename[len(prefix):])
  return path

def process_file(filename, args):
  if not [ext for ext in EXTENSION_RULES.keys() if filename.endswith(ext)]:
    return
  with open(filename) as f:
    content = f.readlines()

  originalContent = list(content)
  if args.tabs:
    content = replace_tabs(content)
  content = replace_rules(filename, content, args.mode == 'carto2nuti')

  originalFilename = filename;
  filename = process_filename(filename, args.mode == 'carto2nuti')

  if content != originalContent or filename != originalFilename:
    with open('%s.bak' % originalFilename, 'w') as f:
      f.writelines(originalContent)
    with open(filename, 'w') as f:
      f.writelines(content)
  if filename != originalFilename:
    os.remove(originalFilename)

parser = argparse.ArgumentParser()
parser.add_argument(dest='name', help='name of the file to change')
parser.add_argument(dest='mode', choices=['nuti2carto', 'carto2nuti'], help='conversion mode')
parser.add_argument('--tabs', dest='tabs', type=int, default=0, help='replace tabs')

args = parser.parse_args()
if os.path.isdir(args.name):
  for root, dirs, files in os.walk(args.name, topdown=True):
    for filename in files:
      if re.search("[\\/][.]git[\\/]", filename) or re.search("[\\/][.]libs[\\/]", filename):
        continue
      process_file(os.path.join(root, filename), args)
else:
  process_file(args.name, args)
