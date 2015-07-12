import Options
from os import unlink, symlink, popen
from os.path import exists

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "js0n-cache"
  obj.source = ["src/js0n-cache.cpp", "src/js0n.cpp"]
  obj.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE"]

def shutdown():
  if Options.commands['clean']:
    if exists('js0n-cache.node'): unlink('js0n-cache.node')
  else:
    if exists('build/default/js0n-cache.node') and not exists('js0n-cache.node'):
      symlink('build/default/js0n-cache.node', 'js0n-cache.node')

