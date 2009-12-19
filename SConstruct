import glob
import excons

prjs = [
  { "name"    : "rex",
    "type"    : "staticlib",
    "defs"    : ["SUBEXPOPTS"],
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/lib/*.cpp"),
  },
  { "name"    : "tests",
    "type"    : "testprograms",
    "incdirs" : ["include"],
    "srcs"    : glob.glob("src/tests/*.cpp"),
    "libs"    : ["rex"]
  }
]

env = excons.MakeBaseEnv()
excons.DeclareTargets(env, prjs)




