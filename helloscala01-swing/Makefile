helloworld.jar: package

run: package
	java -jar helloworld.jar

compile: bin
	scalac -sourcepath src -d bin src/foo/HelloWorld.scala

package: compile
	cd bin && jar -cfm ../helloworld.jar ../MANIFEST.MF ./* && cd ..

bin:
	mkdir bin
