# -*- coding: utf-8 -*- 

def update(con, cur):
    cur.execute("DROP TABLE IF EXISTS flag_explain")

    cur.execute("""CREATE TABLE flag_explain (
           flag INTEGER,
           flagvalue INTEGER,
           language CHAR(2) DEFAULT 'nb',
           description VARCHAR(1024)
    )""")

    flag_explanations = [
    # fagg
        ( 0,  0, u'Kvalitetsinformasjon er ikke gitt'),
        ( 0,  1, u'Grunnlagsdata er funnet i orden'),
        ( 0,  2, u'Grunnlagsdata er litt mistenkelig'),
        ( 0,  3, u'Grunnlagsdata er svært mistenkelig'),
        ( 0,  4, u'Grunnlagsdata er manuelt korrigert'),
        ( 0,  5, u'Grunnlagsdata er manuelt interpolert'),
        ( 0,  6, u'Grunnlagsdata er automatisk korrigert'),
        ( 0,  7, u'Grunnlagsdata er automatisk interpolert'),
        ( 0,  8, u'Grunnlagsdata er manuelt tilfordelt fra akkumulert verdi'),
        ( 0,  9, u'Grunnlagsdata er automatisk tilfordelt fra akkumulert verdi'),
        ( 0, 11, u'Grunnlagsdata er forkastet'),
    # frange
        ( 1,  0, u'Ikke kontrollert'),
        ( 1,  1, u'Kontrollert. Funnet i orden'),
        ( 1,  2, u'Kontrollert. Observert verdi større enn høy testverdi'),
        ( 1,  3, u'Kontrollert. Observert verdi mindre enn lav testverdi'),
        ( 1,  4, u'Kontrollert. Observert verdi større enn høyeste testverdi'),
        ( 1,  5, u'Kontrollert. Observert verdi mindre enn minste testverdi'),
        ( 1,  6, u'Forkastet. Observert verdi utenfor fysikalske grenser'),
        ( 1,  7, u'Kontrollert. Funnet å svare til spesialverdi som betyr manglende'),
        ( 1, 10, u'Kontrollert. Observert verdi utenfor fysikalske grenser. Korrigert automatisk'),
    # fcc
        ( 2,  0, u'Ikke kontrollert.'),
        ( 2,  1, u'Kontrollert. Funnet i orden.'),
        ( 2,  2, u'Kontrollert. Formell inkonsistens, men neppe feil i aktuell parameter.'),
        ( 2,  3, u'Kontrollert. Formell inkonsistens ved observasjonsterminen, men ikke mulig å avgjøre i hvilken parameter.'),
        ( 2,  4, u'Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin, men ikke mulig å avgjøre i hvilken parameter.'),
        ( 2,  6, u'Kontrollert. Formell inkonsistens ved observasjonsterminen, antagelig feil i aktuell parameter.'),
        ( 2,  7, u'Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin, antagelig feil i aktuell parameter.'),
        ( 2,  8, u'Kontrollert. Formell inkonsistens ved observasjonsterminen. En av parametrene mangler.'),
        ( 2,  9, u'Original verdi mangler. Interpolert fra andre parametere.'),
        ( 2, 10, u'Kontrollert. Formell inkonsistens ved observasjonsterminen. Korrigert automatisk.'),
        ( 2, 11, u'Kontrollert. Formell inkonsistens i forhold til tidligere/senere observasjonstermin. Korrigert automatisk.'),
        ( 0, 13, u'Forkastet. Formell inkonsistens.'),
    # fs
        ( 3,  0, u'Ikke kontrollert'),
        ( 3,  1, u'Kontrollert. Funnet i orden'),
        ( 3,  2, u'Kontrollert. Observert endring høyere enn (minste) testverdi'),
        ( 3,  3, u'Kontrollert. Ingen endring i måleverdi over x tidsskritt'),
        ( 3,  4, u'Kontrollert. Mistanke om feil detektert i QC1-3, mistanken opphevet av dipptest i QC2d-1'),
        ( 3,  7, u'Kontrollert. Observert drift i instrumentet'),
        ( 3,  8, u'Observert endring høyere enn høyeste testverdi. Forkastet'),
        ( 3,  9, u'Observert endring høyere enn testverdi. Korrigert automatisk'),
        ( 3, 10, u'Ingen endring i måleverdi over x tidsskritt. Korrigert automatisk'),
    # fnum
        ( 4,  0, u'Ikke kontrollert'),
        ( 4,  1, u'Kontrollert. Funnet i orden'),
        ( 4,  2, u'Kontrollert. Observert verdis avvik fra modellverdien større enn høy testverdi'),
        ( 4,  3, u'Kontrollert. Observert verdis avvik fra modellverdien mindre enn lav testverdi'),
        ( 4,  4, u'Kontrollert. Observert verdis avvik fra modellverdien større enn høyeste testverdi'),
        ( 4,  5, u'Kontrollert. Observert verdis avvik fra modellverdien mindre enn minste testverdi'),
        ( 4,  6, u'Original verdi mangler eller er forkastet. Interpolert/korrigert med modellverdi.'),
    # fpos
        ( 5,  0, u'Ikke kontrollert'),
        ( 5,  1, u'Kontrollert funnet i orden'),
        ( 5,  3, u'Kontrollert. Mistenkelig melding. Ingen korreksjon.'),
        ( 5,  4, u'Kontrollert. Mistenkelig melding. Korrigert automatisk.'),
        ( 5,  6, u'Meldingsverdi forkastet.'),
    # fmis
        ( 6,  0, u'Original verdi eksisterer, det er ikke grunnlag for å si at den er sikkert feilaktig (corrected=original)'),
        ( 6,  1, u'Original verdi mangler. Korrigert verdi eksisterer'),
        ( 6,  2, u'Korrigert verdi mangler. Original verdi eksisterer (original verdi er forkastet)'),
        ( 6,  3, u'Original verdi og korrigert verdi mangler'),
        ( 6,  4, u'Original og korrigert verdi eksisterer, men original er sikkert feilaktig'),
    # ftime
        ( 7,  0, u'Ikke kontrollert'),
        ( 7,  1, u'Interpolert/korrigert med godt resultat'),
        ( 7,  2, u'Interpolert/korrigert med usikkert resultat'),
        ( 7,  3, u'Forsøkt interpolert/korrigert. Metode uegnet.'),
    # fw
        ( 8,  0, u'Ikke kontrollert'),
        ( 8,  1, u'Kontrollert. Funnet i orden'),
        ( 8,  2, u'Kontrollert. Funnet litt mistenkelig'),
        ( 8,  3, u'Kontrollert. Funnet svært mistenkelig'),
        ( 8, 10, u'Forkastet'),
    # fstat
        ( 9,  0, u'Ikke kontrollert'),
        ( 9,  1, u'Kontrollert. Funnet i orden'),
        ( 9,  2, u'Kontrollert. Mistenkelig verdi. Ingen korreksjon.'),
    # fcp
        (10,  0, u'Ikke kontrollert'),
        (10,  1, u'Kontrollert. Funnet i orden'),
        (10,  2, u'Kontrollert. Klimatologisk tvilsom, men neppe feil i aktuell parameter'),
        (10,  3, u'Kontrollert. Klimatologisk tvilsom ved observasjonsterminen, men ikke mulig å avgjøre i hvilken parameter'),
        (10,  4, u'Kontrollert. Klimatologisk tvilsom i forhold til tidligere/senere observasjonstermin, men ikke mulig å avgjøre i hvilken parameter'),
        (10,  6, u'Kontrollert. Klimatologisk tvilsom ved observasjonsterminen, antagelig feil i aktuell parameter'),
        (10,  7, u'Kontrollert. Klimatologisk tvilsom i forhold til tidligere/senere observasjonstermin, antagelig feil i aktuell parameter'),
        (10, 10, u'Kontrollert. Klimatologisk inkonsistens ved observasjonsterminen. Korrigert automatisk.'),
        (10, 11, u'Kontrollert. Klimatologisk inkonsistens i forhold til tidligere/senere observasjonstermin. Korrigert automatisk.'),
    # fclim
        (11,  0, u'Ikke kontrollert'),
        (11,  1, u'Kontrollert. Funnet i orden'),
        (11,  2, u'Kontrollert. Mistenkelig verdi. Ikke korrigert'),
        (11,  3, u'Kontrollert. Mistenkelig verdi. Korrigert automatisk'),
    # fd
        (12,  0, u'Ikke vurdert som samleverdi'),
        (12,  1, u'Normal observasjon, ikke oppsamlet verdi'),
        (12,  2, u'Oppsamling pågår'),
        (12,  3, u'Unormal observasjon. Original verdi kan være oppsamlet verdi'),
        (12,  4, u'Oppsamlet verdi før tilfordeling'),
        (12,  7, u'Korrigert verdi er et resultat av automatisk tilfordeling. Original verdi mangler eller er forkastet'),
        (12,  8, u'Original verdi er oppsamling, korrigert verdi er tilfordelt automatisk'),
        (12,  9, u'Korrigert verdi er et resultat av manuell tilfordeling. Original verdi mangler eller er forkastet'),
        (12, 10, u'Original verdi er oppsamling, korrigert verdi er tilfordelt manuellt'),
    # fpre
        (13,  0, u'Ikke vurdert'),
        (13,  1, u'Vurdert. Ikke forkastet'),
        (13,  4, u'Korrigert automatisk'),
        (13,  6, u'Forkastet. Original verdi er kjent å være feil'),
        (13,  7, u'Forkastet. Parameter utelatt fra ny innsendt melding'),
    # fcombi
        (14,  0, u'Ikke tolket'),
        (14,  1, u'Tolket. Funnet iorden.'),
        (14,  2, u'Utenfor høyeste grenseverdi, ikke påvist sprang, innenfor tolerert avvik fra modellverdi'),
        (14,  9, u'Forkastet. Utenfor høyeste grenseverdi, ikke påvist sprang, utenfor tolerert avvik fra modellverdi'),
        (14, 10, u'Forkastet. Utenfor høyeste grenseverdi, påvist sprang, innenfor tolerert avvik fra modellverdi'),
        (14, 11, u'Forkastet. Utenfor høyeste grenseverdi, påvist sprang, utenfor tolerert avvik fra modellverdi'),
    # fhqc
        (15,  0, u'Ikke kontrollert i HQC'),
        (15,  1, u'Kontrollert i HQC. Funnet i orden.'),
        (15,  2, u'Ikke kvalifisert for feilliste. Sannsynligvis i orden.'),
        (15,  3, u'Ikke ferdig HQC kontrollert. Mulig kvalifisert for feilliste. Ikke aktuell for kontroll i QC1 eller QC2.'),
        (15,  4, u'Ikke ferdig HQC kontrollert. Mulig kvalifisert for feilliste. Aktuell for interpolering i QC2.'),
        (15,  5, u'Interpolert manuelt'),
        (15,  6, u'Tilfordelt manuelt'),
        (15,  7, u'Korrigert manuelt'),
        (15, 10, u'Forkastet manuelt')
    ]

    for fe in flag_explanations:
        cur.execute("INSERT INTO flag_explain(flag, flagvalue, description) VALUES (?, ?, ?)", fe)
