TEMPLATE = subdirs
SUBDIRS = src \
          #plugins/runner \
          # plugins/weby


win32 {
        #SUBDIRS += plugins/controly
        #SUBDIRS += plugins/calcy
}

TRANSLATIONS = translations/launchy_fr.ts \
    translations/launchy_nl.ts \
    translations/launchy_zh.ts \
    translations/launchy_es.ts \
    translations/launchy_de.ts \
    translations/launchy_ja.ts \
    translations/launchy_zh_TW.ts \
    translations/launchy_rus.ts \
    translations/launchy_it_IT.ts \
