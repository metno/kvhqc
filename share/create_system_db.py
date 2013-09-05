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

dbfile = sys.argv[1]
try:
    os.remove(dbfile)
except:
    pass

con = sqlite3.connect(dbfile)
cur = con.cursor()

for m in modules:
    print m.__name__
    m.update(con, cur)

cur.close()
con.commit()
con.close()
