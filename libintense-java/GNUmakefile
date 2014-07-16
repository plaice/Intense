# *****************************************************************************
#
# GNU Makefile for libintense (Java version)
#
# Paul Swoboda, 2002, 2004
#
# *****************************************************************************

default : jar

# *****************************************************************************
# begin configuration/non-standard section

JAVAC = javac

PACKAGES = intense intense.log intense.aep

JARNAME = intense.jar

CLASSPATH = src

JAVAC_DEST_ROOT = classes

JAVAC_SRC_ROOT = src

INTERMEDIATE_SRCS = src/intense/ContextOpLexer.java \
                       src/intense/aep/AETPLexer.java

EXTRA_JAVAC_SRC_FILES = $(INTERMEDIATE_SRCS)

src/intense/ContextOpLexer.java : src/intense/ContextOp.jflex
	cd src/intense; java -classpath ../../ext/JFlex.jar JFlex.Main \
        ContextOp.jflex

src/intense/aep/AETPLexer.java : src/intense/aep/AETP.jflex
	cd src/intense/aep; java -classpath ../../../ext/JFlex.jar JFlex.Main \
        AETP.jflex

# end configuration/non-standard section
# *****************************************************************************

PACKAGE_PATHS = $(subst .,/, $(PACKAGES))

PACKAGE_PATH_ROOTS = $(sort $(foreach path, $(PACKAGE_PATHS), \
                              $(firstword $(subst /, , $(path)))))

JAVAC_SRC_DIRS = $(foreach package, $(PACKAGE_PATHS), \
                      $(JAVAC_SRC_ROOT)/$(package))

JAVAC_SRC_FILES = $(wildcard \
                       $(foreach package_dir, $(JAVAC_SRC_DIRS), \
                         $(package_dir)/*.java)) \
                     $(EXTRA_JAVAC_SRC_FILES)

JAVAC_FLAGS = -classpath $(CLASSPATH) -g

JAVAC_DEST_FILES = $(patsubst $(JAVAC_SRC_ROOT)/%, $(JAVAC_DEST_ROOT)/%, \
                     $(patsubst %.java, %.class, $(JAVAC_SRC_FILES)))

JAR = ./$(JARNAME)

.SECONDARY : $(JAVAC_DEST_FILES)

$(JAVAC_DEST_ROOT)/%.class : $(JAVAC_SRC_ROOT)/%.java $(INTERMEDIATE_SRCS)
	$(JAVAC) $(JAVAC_FLAGS) -d $(JAVAC_DEST_ROOT) $<

targets : $(JAR)

.lib : $(JAVAC_DEST_FILES)
	touch .lib

lib : .lib

jar : $(JAR)

$(JAR) : $(JAVAC_DEST_FILES)
	jar cvf $(JAR) $(foreach root, $(PACKAGE_PATH_ROOTS), \
                         -C $(JAVAC_DEST_ROOT) $(root))

check : jar
	cd tests; make

javadocs : doc/.stampdoc

doc/.stampdoc : $(JAVAC_SRC_FILES)
	javadoc -d doc -sourcepath src $(PACKAGES)
	touch doc/.stampdoc

clean :
	rm -rf $(JAR) .lib
	mv $(JAVAC_DEST_ROOT)/CVS ./tmpCVS
	rm -rf $(JAVAC_DEST_ROOT)/*
	mv ./tmpCVS $(JAVAC_DEST_ROOT)/CVS
	cd tests; make clean
	mv doc/CVS ./tmpCVS
	rm -rf doc/*
	mv ./tmpCVS doc/CVS

distclean : clean
