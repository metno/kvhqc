#!/usr/bin/env python
# -*- coding: utf-8 -*- 

import os.path, pkgutil, sqlite3, sys

sys.dont_write_bytecode = True

def load_all_modules_from_dir(dirname):
    # based on http://stackoverflow.com/a/8556471
    modules = []
    for importer, package_name, _ in pkgutil.iter_modules([dirname]):
        full_package_name = '%s.%s' % (dirname, package_name)
        if full_package_name not in sys.modules:
            module = importer.find_module(package_name).load_module(full_package_name)
            modules.append(module)
    return modules

modules = load_all_modules_from_dir(os.path.join(os.path.dirname(sys.argv[0]), 'system_db'))

class SystemDB:
    def __init__(self, dbfile):
        self.paramids = { }
        self.__read_paramids()
        self.con = sqlite3.connect(dbfile)
        self.cur = self.con.cursor()

    def close(self):
        self.cur.close()
        self.con.commit()
        self.con.close()

    def paramid(self, name):
        return self.paramids[name]

    def __read_paramids(self):
        # update paramids.txt with:
        #   psql -c "select name, paramid from param order by name" > share/system_db/paramids.txt
        pids = open(os.path.join(os.path.dirname(sys.argv[0]), 'system_db', 'paramids.txt'), 'r')
        pids.readline()
        pids.readline()
        for line in pids:
            ls = line.split('|')
            if len(ls) == 2:
                name = ls[0].strip()
                pid = int(ls[1].strip())
                self.paramids[name] = pid
        pids.close()

dbfile = sys.argv[1]
try:
    os.remove(dbfile)
except:
    pass

sdb = SystemDB(dbfile)

for m in modules:
    print m.__name__
    m.update(sdb)

sdb.close()
