
import ctypes
import email.mime.text
import errno
import fnmatch
import os
import platform
import shutil
import smtplib
import stat
import subprocess
import sys
import time


def getFreeSpace(folder):

    # Return folder/drive free space (in bytes)

    if platform.system() == 'Windows':
        freeBytes = ctypes.c_ulonglong(0)
        ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p(folder), None, None, ctypes.pointer(freeBytes))
        return freeBytes.value
    else:
        s = os.statvfs(folder)
        return s.f_bavail * s.f_bsize


def getLowestBuildDir(baseFolder, postfix):

    # find the folder with the lowest build number which also has the matching postfix

    lowestbuild = 999999
    lowestbuilddir = ""

    for dir in os.listdir(baseFolder):
        if os.path.isdir(os.path.join(baseFolder, dir)):
            m = re.search('-{0}\Z'.format(postfix), dir)
            if m is None:
               continue;

            m = re.search('\S*Build(\d+)-\S+', dir)
            if m is not None:
                buildnum = int(m.group(1))
                if buildnum < lowestbuild:
                    lowestbuild = buildnum
                    lowestbuilddir = dir

    return lowestbuilddir


def findSVNInfo(svnHost, svnUser, svnPassword, svnInfo):

    # run svn info, looking for the specified line of info and return it

    p = subprocess.Popen("svn info --non-interactive --username {0} --password {1} {2}".format(svnUser, svnPassword, svnHost).split(" "), stdout=subprocess.PIPE)
    svnOutput = []
    for line in p.stdout:
        svnOutput.append(line.strip())
    p.wait()

    for line in svnOutput:
        if (line.startswith(svnInfo)):
            return line[len(svnInfo):]

    return ""


def handleRemoveReadonly(func, path, exc):

    # needed for rmtree()

    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO) # 0777
        func(path)
    else:
        raise


def intWithCommas(x):

    # http://stackoverflow.com/questions/1823058/how-to-print-number-with-commas-as-thousands-separators-in-python-2-x

    if type(x) not in [type(0), type(0L)]:
        raise TypeError("Parameter must be an integer.")
    if x < 0:
        return '-' + intWithCommas(-x)
    result = ''
    while x >= 1000:
        x, r = divmod(x, 1000)
        result = ",%03d%s" % (r, result)
    return "%d%s" % (x, result)


def handleRemoveReadonly(func, path, exc):

    # needed for rmtree()

    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO) # 0777
        func(path)
    else:
        raise



def deleteDir(folder, excludes=None):
    # if no exclusion set, shortcut the slow method
    if excludes is None:
        if os.path.exists(folder):
            shutil.rmtree(folder, onerror=handleRemoveReadonly)
        return

    # delete the file unless it matches the exclusion
    for base, dirs, files in os.walk(folder):
        for file in files:
            if not fnmatch.fnmatch(file, excludes):
                os.remove(os.path.join(base, file))


    # delete empty folders  (horribly inefficient)
    folderFound = True
    while folderFound:
        folderFound = False
        for base, dirs, files in os.walk(folder):
            if len(dirs) == 0 and \
               len(files) == 0:
                os.rmdir(base)

                folderFound = True


def deleteFiles(folder, wildcard):
    # recursively delete all files that match the wildcard
    for base, dirs, files in os.walk(folder):
        for file in files:
            if fnmatch.fnmatch(file, wildcard):
                os.remove(os.path.join(base, file))



def cleanBuild():
    # Check to see if a build is already running
    if os.path.exists("BUILD_RUNNING"):
       print "Build is already running.  Exiting...\n";
       sys.exit()

    if os.path.exists("build"):
        shutil.rmtree("build", onerror=handleRemoveReadonly)


def cleanSandbox():
    cleanBuild()

    # Check to see if a build is already running
    if os.path.exists("BUILD_RUNNING"):
       print "Build is already running.  Exiting...\n";
       sys.exit()

    print "Removing build.sandbox folder to make way for a clean checkout"

    if os.path.exists("build.sandbox"):
        shutil.rmtree("build.sandbox", onerror=handleRemoveReadonly)


def makeDist(distLocation):
    # This builds a distribution, not in the traditional sense, but rather takes a build and removes unneeded files to produce a 'clean' build.

    print "Removing source and extra data from: {0}".format(distLocation)


def fullBuild(svnPassword, buildSuffix):

    """
    This script performs the following tasks:
        - check to see if a previous build is running, and quits if so
        - create a timestamp file to indicate when this build has run
        - svn checkout (or update) from repository to sandbox directory
        - export sandbox to build directory
        - compile project
        - move the build to final destination
        - gather directory statistics
        - filter revision numbers from svn output
        - filter errors and warnings from compiler output
        - generate email report
        - tag repository with email report
        - generate build output
    """


    svnUsername = "iabuild"
    destinationFolder = ""
    if os.name == "nt":
        destinationFolder = "D:/SBM-Builds"
    else:
        destinationFolder = "/Users/fast/build/sbm-builds"
    tagSvn = False
    emailReport = True


    cleanBuild()


    totalBuildTime = time.clock()


    # Check to see if a build is already running
    if os.path.exists("BUILD_RUNNING"):
       print "Build is already running.  Exiting...\n";
       sys.exit()


    # Create the build running file, and update the timestamp file
    open("BUILD_RUNNING", "w").close()
    f = open("BUILD_TIME", "w")
    f.write(time.strftime("%Y-%m-%d %H:%M:%S"))
    f.close()


    # checkout/update build from svn into sandbox
    print "--- Starting build.svn.checkout..."

    buildSvnTime = time.clock()

    buildSvnCleanCheckout = False

    if os.path.exists("build.sandbox"):
        p = subprocess.Popen("svn update --non-interactive --username {0} --password {1} build.sandbox".format(svnUsername, svnPassword).split(" "), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        buildSvnOutput = []
        for line in p.stdout:
            buildSvnOutput.append(line.strip())
        p.wait()
    else:
        p = subprocess.Popen("svn checkout --non-interactive --username {0} --password {1} https://smartbody.svn.sourceforge.net/svnroot/smartbody/trunk build.sandbox".format(svnUsername, svnPassword).split(" "), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        buildSvnOutput = []
        for line in p.stdout:
            buildSvnOutput.append(line.strip())
        p.wait()

        buildSvnCleanCheckout = True

    buildSvnTime = time.clock() - buildSvnTime



    print "--- Starting build.svn..."

    buildExportTime = time.clock()

    p = subprocess.Popen("svn export --non-interactive --username {0} --password {1} build.sandbox build".format(svnUsername, svnPassword).split(" "), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    buildSvnExportOutput = []
    for line in p.stdout:
        buildSvnExportOutput.append(line.strip())
    p.wait()

    buildExportTime = time.clock() - buildExportTime



    print "--- Starting build.compile..."

    buildCompileTime = time.clock()

    currentWorkingDir = os.getcwd()
    p = None
    if os.name == "nt":
        os.chdir("build")
        p = subprocess.Popen("compile-sbm.bat", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    else:
        p = subprocess.Popen("./sb-compile.sh", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    buildCompileOutput = []
    for line in p.stdout:
        buildCompileOutput.append(line.strip())
    p.wait()
    if os.name == "nt":
        os.chdir(currentWorkingDir)

    buildCompileTime = time.clock() - buildCompileTime



    # filter out revision numbers from svn output

    print "--- Analyzing output..."

    buildSvnRevisions = []
    for line in buildSvnOutput:
        # update vs checkout.  the output is different

        if line.startswith("Checked out revision ") or        \
           line.startswith("Checked out external at "):
           buildSvnRevisions.append("   " + line)

        if line.startswith("Updated external to revision ") or \
           line.startswith("Fetching external item into ") or  \
           line.startswith("Updated to revision ") or          \
           line.startswith("External at revision ") or         \
           line.startswith("At revision "):
           buildSvnRevisions.append("   " + line)


    # filter the final revision number

    buildSvnRevision = buildSvnRevisions[len(buildSvnRevisions) - 1]
    buildSvnRevision = ''.join(i for i in buildSvnRevision if i.isdigit())  # only grab the digits on this line
    buildSvnRevision = int(buildSvnRevision)

    print "Build revision: {0}".format(buildSvnRevision)


    # filter out compiler errors and warnings

    #  ": warning "
    #  ": error "
    #  ": fatal error "
    #  ": warning: "   # gcc
    #  ": error: "     # gcc

    buildCompileErrors = []
    buildCompileWarnings = []
    for line in buildCompileOutput:
        if ": error " in line or \
           ": fatal error " in line or \
           ": error: " in line:
           buildCompileErrors.append("   " + line)

        if ": warning " in line or \
           ": warning: " in line:
           buildCompileWarnings.append("   " + line)



    # determine if build succeeded by looking for compiler errors

    if len(buildCompileErrors) == 0:
        buildSuccess = True
        print "Build Success."
    else:
        buildSuccess = False
        print "Build Failed."


    # Get time of last build

    if os.path.exists("BUILD_NUMBER"):
        f = open("BUILD_NUMBER","r")
        buildNumber = int(f.read())
        f.close()
    else:
        buildNumber = 0

    buildNumber += 1

    f = open("BUILD_NUMBER","w")
    f.write(str(buildNumber))
    f.close()

    print "build: {0}".format(buildNumber)


    buildDate = time.strftime("%m-%d-%Y")

    print "date: {0}".format(buildDate)


    if buildSuccess:
        buildFolderName = "Build{0}-{1}{2}".format(buildNumber, buildDate, buildSuffix)
    else:
        buildFolderName = "Build{0}-{1}{2}-failed".format(buildNumber, buildDate, buildSuffix)

    print "buildFolderName: {0}".format(buildFolderName)


    buildFolder = os.path.join(destinationFolder, buildFolderName)

    print "buildFolder: {0}".format(buildFolder)


    # move build to its destination folder
    os.makedirs(buildFolder)
    shutil.move("build", os.path.join(buildFolder, "smartbody"))


    print "--- Starting directory statistics..."
    dirSizeTotal = 0
    numFilesTotal = 0
    numDirsTotal = 0
    for dirpath, dirnames, filenames in os.walk(buildFolder):
        for filename in filenames:
            if os.name == "nt":
                dirSizeTotal += os.path.getsize(os.path.join(dirpath,filename))
            numFilesTotal += 1

        for dirname in dirnames:
            numDirsTotal += 1

    print "size:  {0}".format(dirSizeTotal)
    print "files: {0}".format(numFilesTotal)
    print "dirs:  {0}".format(numDirsTotal)


    totalBuildTime = time.clock() - totalBuildTime


    totalBuildTime_t   = time.gmtime(totalBuildTime)
    buildSvnTime_t     = time.gmtime(buildSvnTime)
    buildExportTime_t  = time.gmtime(buildExportTime)
    buildCompileTime_t = time.gmtime(buildCompileTime)


    # generate output to be used in email report

    finalMailFile = os.path.join(buildFolder, "FinalMail.txt")

    buildServerShareName = ""
    if os.name == "nt":
        buildServerShareName = "\\\\vhbuild\\SBM-builds\\"
        emailSubjectPrefix = "[SB]"
    else:
        buildServerShareName = "\\\\roscoemini\\sbm-builds\\"
        emailSubjectPrefix = "[SB-MAC]"

    f = open(finalMailFile,"w")
    f.write("build: r{0} as Build #{1}\n".format(buildSvnRevision, buildNumber))
    f.write("\n")
    f.write("Build Summary:\n")
    if buildSuccess:
        f.write("   Build Success.\n")
    else:
        f.write("   Build FAILED!!\n")
    f.write("\n")
    f.write("Broken projects:\n")
    for line in buildCompileErrors:
        f.write(line + "\n")
    f.write("\n")
    f.write("Dirty projects:\n")
    for line in buildCompileWarnings:
        f.write(line + "\n")
    f.write("\n")
    f.write("\n")
    f.write("Build {0} took {1}\n".format(buildNumber, time.strftime("%X", totalBuildTime_t)))
    f.write("   buildSvnTime {0}\n".format(time.strftime("%X", buildSvnTime_t)))
    f.write("   buildExportTime {0}\n".format(time.strftime("%X", buildExportTime_t)))
    f.write("   buildCompileTime {0}\n".format(time.strftime("%X", buildCompileTime_t)))
    f.write("\n")
    f.write("Build size: {0} ({1} files, {2} dirs)\n".format(intWithCommas(dirSizeTotal), intWithCommas(numFilesTotal), intWithCommas(numDirsTotal)))
    f.write("\n")
    f.write("Build is available at {0}{1}\n".format(buildServerShareName, buildFolderName))
    f.write("\n")
    f.write("Log: {0}{1}{2}\n".format(buildServerShareName, buildFolderName, "\\BuildLog.txt"))
    f.write("\n")
    f.write("Revisions used:\n")
    f.write("sb revisions (based off of r{0}):\n".format(buildSvnRevision))
    for line in buildSvnRevisions:
        f.write(line + "\n")
    f.close()


    # tag the repository with the email report
    if tagSvn:
        p = subprocess.Popen("svn copy --non-interactive --username {0} --password {1} -r {2} {3} {4}/{5} -F ""{6}""".format(svnUsername, svnPassword, buildSvnRevision, "https://svn.ict.usc.edu/svn_repo/trunk", "https://svn.ict.usc.edu/svn_repo/tags/builds", buildFolderName, finalMailFile).split(" "))
        p.wait()

    if emailReport:
        fp = open(finalMailFile, 'r')
        msg = email.mime.text.MIMEText(fp.read())
        fp.close()

        emailTo = "nospam@ict.usc.edu"
        msg["Subject"] = "{0} Build Results #{1} - r{2}".format(emailSubjectPrefix, buildNumber, buildSvnRevision)
        msg["From"] = "svn@ict.usc.edu"
        msg["To"] = emailTo

        s = smtplib.SMTP("smtp.ict.usc.edu")
        s.sendmail("svn@ict.usc.edu", emailTo.split(";"), msg.as_string())
        s.quit()


    ## os.remove(finalMailFile)


    # generate output for log file
    buildLogFile = os.path.join(buildFolder, "BuildLog.txt")

    f = open(buildLogFile,"w")
    f.write("------------------\n")
    f.write("\n")
    for line in buildSvnOutput:
        f.write(line + "\n")
    f.write("\n")
    f.write("\n")
    f.write("------------------\n")
    f.write("\n")
    for line in buildSvnExportOutput:
        f.write(line + "\n")
    f.write("\n")
    f.write("\n")
    f.write("------------------\n")
    f.write("\n")
    for line in buildCompileOutput:
        f.write(line + "\n")
    f.write("\n")
    f.write("\n")
    f.write("------------------\n")


    if os.path.exists("BUILD_RUNNING"):
        os.remove("BUILD_RUNNING")


    distLocation = buildFolder


    makeDist(distLocation)


def checkIfTimeForBuild(svnURL, svnUser, svnPassword, minFreeSpaceRequiredForBuildGig, buildDrive, buildOutputFolder, buildSuffix):

    """
    This script:

    - checks to see if BUILD_RUNNING file exists, exits if so
    - gets the date/time of the last build from BUILD_TIME file
    - runs 'svn info' to get the date/time of the last commit
    - figures out if it's time to run a new build based on the date/times
    - if there isn't enough disk space, deletes folders ending with -ci, starting with the oldest build
    - runs the given build script

    """

    # Check to see if a build is already running
    if os.path.exists("BUILD_RUNNING"):
       print "Build is already running.  Exiting...\n";
       sys.exit()


    # Get time of last build
    if os.path.exists("BUILD_TIME"):
        f = open("BUILD_TIME","r")
        lastBuildDate = f.read()
        f.close()
    else:
        lastBuildDate = "1980-01-01 00:00:01"


    # Get time of last revision from the repository
    lastChangedDate = findSVNInfo( svnURL, svnUser, svnPassword, "Last Changed Date: " )[0:19]

    lastBuildDate_t = time.strptime(lastBuildDate, "%Y-%m-%d %H:%M:%S")
    lastChangedDate_t = time.strptime(lastChangedDate, "%Y-%m-%d %H:%M:%S")


    print "Last Build Date:   {0}".format(time.strftime("%c", lastBuildDate_t))
    print "Last Changed Date: {0}".format(time.strftime("%c", lastChangedDate_t))
    print "Current Time:      {0}".format(time.strftime("%c", time.localtime()))


    # figure out if it's time to run a build
    timeToWaitSinceLastCommit    = 5 * 60   # in minutes * 60
    timeSinceLastBuild           = 10       # in seconds

    doBuild = False

    if time.mktime(time.localtime()) - time.mktime(lastChangedDate_t) < timeToWaitSinceLastCommit:
        print "Too soon since last commit"
    else:
        if time.mktime(lastChangedDate_t) - time.mktime(lastBuildDate_t) > timeSinceLastBuild:
            doBuild = True
        else:
            print "Too soon since last build"


    if not doBuild:
        sys.exit()



    print "Time to run a new build"


    # see how much disk space is left
    # find all dirs that end with -ci
    # find lowest build num
    # delete build
    # loop until enough space is left

    if os.name == "nt":
        minFreeSpaceRequiredForBuild = minFreeSpaceRequiredForBuildGig * 1000 * 1000 * 1000;  # ~gigs

        while getFreeSpace(buildDrive) < minFreeSpaceRequiredForBuild:
            lowestBuildDir = getLowestBuildDir(buildOutputFolder, "ci")

            if lowestBuildDir == "":
                 print "No CI build dirs found.  Can't delete enough hard disk space.\n"
                 sys.exit()

            lowestBuildFullPath = os.path.join(buildOutputFolder, lowestBuildDir)

            print "Removing folder {0} to free up disk space".format(lowestBuildFullPath)
            shutil.rmtree(lowestBuildFullPath, onerror=handleRemoveReadonly)


    print "Disk has enough space for build\n";


    # start the build
    fullBuild(svnPassword, buildSuffix)


def printUsage():
    print ""
    print "Usage:  python create-build.py [task] [args]"
    print "   Where task is:"
    print "     python create-build.py build [svnPassword] [buildSuffix]"
    print "            # performs the full build, both arguments optional, default to \"\""
    print "     python create-build.py cleansandbox"
    print "            # wipes out the svn sandbox to do a fresh checkout"
    print "     python create-build.py checktime [svnURL] [svnUser] [svnPassword]"
    print "            [minSpace] [buildDrive] [buildOutputFolder] [buildSuffix]"
    print "            # determines if time to run a new build, then triggers the build"
    print "            eg:"
    print "            python create-build.py checktime \\"
    print "            https://svn.ict.usc.edu/svn_vhtoolkit/trunk IABuild passwordblah \\"
    print "            200 d: d:/VHToolkit-Builds -ci"


##############################################################


if len(sys.argv) <= 1:
    printUsage()
else:
    if sys.argv[1] == "build":
        svnPassword = ""
        buildSuffix = ""

        if len(sys.argv) > 2:
            svnPassword = sys.argv[2]

        if len(sys.argv) > 3:
            buildSuffix = sys.argv[3]

        fullBuild(svnPassword, buildSuffix)

    elif sys.argv[1] == "cleansandbox":
        cleanSandbox()
    elif sys.argv[1] == "checktime":
        svnURL = sys.argv[2]
        svnUser = sys.argv[3]
        svnPassword = sys.argv[4]
        minFreeSpaceRequiredForBuildGig = int(sys.argv[5])
        buildDrive = sys.argv[6]
        buildOutputFolder = sys.argv[7]
        buildSuffix = sys.argv[8]
        checkIfTimeForBuild(svnURL, svnUser, svnPassword, minFreeSpaceRequiredForBuildGig, buildDrive, buildOutputFolder, buildSuffix)
    else:
        printUsage()
