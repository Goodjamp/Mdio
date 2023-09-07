import sys
import regex as re
import subprocess

# The version number based on the next data
# - VERSION_MAJOR from the version.h
# - VERSION_MINOR from the version.h
# - VERSION_BUILD from the last tag

try:
    if len(sys.argv) == 2:
        # processing git tags to update biuild number
        tagFile = open(sys.argv[1], "r")
        tagContent = tagFile.read()
        tagFile.close()
        tagList = re.findall(r'\d{1,2}.\d{1,2}.\d{1,3}', tagContent)
        buildNumberInt = 0
        for k in range(0, len(tagList) - 1):
            buildNumberText = re.findall(r'\d{1,3}', tagList[k])
            if buildNumberInt < int(buildNumberText[len(buildNumberText) - 1], 10):
                buildNumberInt = int(buildNumberText[len(buildNumberText) - 1], 10)
        buildNumberInt += 1
        print("buildNumberInt ", buildNumberInt)

        # processing Version.h file.
        # Update build number based on the last tag
        versionFile = open("..\Version.h", "r+")
        versionFileContent = versionFile.read()
        versionMajorText = re.findall(r'\d{1,2}', re.findall(r'#define VERSION_MAJOR\s*\d{1,2}', versionFileContent)[0])
        versionMinorText = re.findall(r'\d{1,2}', re.findall(r'#define VERSION_MINOR\s*\d{1,2}', versionFileContent)[0])
        versionFileContent = re.sub(r'#define VERSION_BUILD\s*\d{1,3}', "#define VERSION_BUILD    " + str(buildNumberInt), versionFileContent)
        versionFile.seek(0)
        versionFile.write(versionFileContent)

        # truncate (or decrease) file size to the current position (becouse I don't pass argument). The current position is equal to the
        # last write position
        versionFile.truncate()
        versionFile.close()

        # Update tag, based on the
        # - VERSION_MAJOR from the version.h
        # - VERSION_MINOR from the version.h
        # - VERSION_BUILD from the last tag
        newVersion = versionMajorText[0] + "." + versionMinorText[0] + "." + str(buildNumberInt)
        print("version: ", newVersion)
        subprocess.run(["git","tag", newVersion])
except:
    print("Can't update version")