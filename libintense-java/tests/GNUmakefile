# Do you have any idea what this file would look like in 'ant'?

JAR = ../intense.jar

CLASSPATH = .:$(JAR)

JAVA = java

JAVAC = javac

JAVA_FLAGS = -classpath $(CLASSPATH)

JAVAC_FLAGS = -classpath $(CLASSPATH) -g

TEST_BASE_CLASSES = IntenseTest.class IntenseTestException.class Test.class

AEP_TEST_BASE_CLASSES = IntenseAEPTest.class Test.class

check : IntenseTest0 IntenseTest1 IntenseTest2 IntenseTest3 IntenseTest4 \
        IntenseTest5 IntenseTest6 IntenseTest7 \
        IntenseAEPTest0 IntenseAEPTest1 IntenseAEPTest2 IntenseAEPTest3
	@echo
	@echo ALL TESTS PASSED
	@echo

IntenseTest% : IntenseTest%.class $(TEST_BASE_CLASSES)
	$(JAVA) $(JAVA_FLAGS) $@

IntenseAEPTest% : IntenseAEPTest%.class $(AEP_TEST_BASE_CLASSES)
	$(JAVA) $(JAVA_FLAGS) $@

%.class : %.java
	$(JAVAC) $(JAVAC_FLAGS) $<

clean :
	rm -f *.class

.PRECIOUS: %.class
