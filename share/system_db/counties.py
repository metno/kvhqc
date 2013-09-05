# -*- coding: utf-8 -*- 

def update(sdb):
    cur = sdb.cur
    cur.execute("DROP TABLE IF EXISTS station_county_labels")
    cur.execute("DROP TABLE IF EXISTS station_counties")
    cur.execute("DROP TABLE IF EXISTS station_region_labels")
    cur.execute("DROP TABLE IF EXISTS station_regions")

    cur.execute("""CREATE TABLE station_regions (
           id INTEGER UNIQUE,
           sortkey INTEGER
    )""")
    cur.execute("""CREATE TABLE station_region_labels (
           region_id INTEGER NOT NULL REFERENCES station_regions(id),
           language CHAR(2) DEFAULT 'nb',
           label VARCHAR(32)
    )""")
    cur.execute("""CREATE TABLE station_counties (
           id INTEGER UNIQUE,
           region_id INTEGER REFERENCES station_regions(id),
           db_name VARCHAR(32),
           sortkey INTEGER
    )""")
    cur.execute("""CREATE TABLE station_county_labels (
           county_id INTEGER NOT NULL REFERENCES station_counties(id),
           language CHAR(2) DEFAULT 'nb',
           label VARCHAR(32)
    )""")

    counties = (
        u"Oslo", u"Akershus", u"Østfold", u"Hedmark", u"Oppland", u"Buskerud", u"Vestfold", u"Telemark", u"Aust-Agder",
        u"Vest-Agder", u"Rogaland", u"Hordaland", u"Sogn og Fjordane",
        u"Møre og Romsdal", u"Sør-Trøndelag", u"Nord-Trøndelag", u"Nordland",
        u"Troms", u"Finnmark", u"Ishavet",
        u"Maritime", u"Skip", u"Andre"
    )
    countiesU = (
        u"OSLO", u"AKERSHUS", u"ØSTFOLD", u"HEDMARK", u"OPPLAND", u"BUSKERUD", u"VESTFOLD", u"TELEMARK", u"AUST-AGDER",
        u"VEST-AGDER", u"ROGALAND", u"HORDALAND", u"SOGN OG FJORDANE",
        u"MØRE OG ROMSDAL", u"SØR-TRØNDELAG", u"NORD-TRØNDELAG", u"NORDLAND",
        u"TROMS", u"FINNMARK", u"ISHAVET",
        u"MARITIME", u"SKIP", u"OTHER"
    )

    region_counties = ( 0, 9, 13, 16, 20, len(counties) )
    regions = (
        u"Østlandet", u"Vestlandet", u"Midt-Norge", u"Nord-Norge", u"Andre"
    )

    for rid in range(len(regions)):
        cur.execute("INSERT INTO station_regions VALUES (?, ?)", (rid, rid))
        cur.execute("INSERT INTO station_region_labels (region_id, label) VALUES (?, ?)", (rid, regions[rid]))
        for cid in range(region_counties[rid], region_counties[rid+1]):
            cur.execute("INSERT INTO station_counties VALUES (?, ?, ?, ?)", (cid, rid, countiesU[cid], cid))
            cur.execute("INSERT INTO station_county_labels (county_id, label) VALUES (?, ?)", (cid, counties[cid]))
