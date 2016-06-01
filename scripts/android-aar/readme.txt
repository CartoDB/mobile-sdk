
# building AAR:

  sh /Applications/Android\ Studio.app/Contents/gradle/gradle-2.4/bin/gradle assemble

# publishing to maven repo as SNAPSHOT:
# requisities:
# 1. pom.xml in same folder, to define webdav plygin:

<project>
   <modelVersion>4.0.0</modelVersion>
   <groupId>fake</groupId>
   <artifactId>fake</artifactId>
   <version>1</version>
 
   <build>
      <extensions>
         <extension>
            <groupId>org.apache.maven.wagon</groupId>
            <artifactId>wagon-webdav-jackrabbit</artifactId>
            <version>1.0-beta-7</version>
         </extension>
      </extensions>
   </build>
 
</project>

# 2. repo user/password in ~/.m2/settings.xml file (user 'nutiteq')
 

# run to upload:
  mvn deploy:deploy-file
   -DgroupId=com.nutiteq
   -DartifactId=nutiteq-sdk
   -Dversion=snapshot
   -Dpackaging=aar
   -Dfile=build/outputs/aar/android-release.aar
   -Durl=dav:https://repository-nutiteq.forge.cloudbees.com/snapshot/
   -DrepositoryId=cloudbees-snapshot