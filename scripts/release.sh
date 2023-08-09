#!/bin/bash
# This script is used to create a new release and trigger the relase workflow
# This will create a new commit with the version and the changelog, tag it and 
# push it to the main branch. After that it will publish the release on the github
# project page.
# This is an interactive script. You don't need to provide any arguments.

while :
do
    echo "Please enter the version as major.minor.patch:"
    read VERSION
    
    if [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        break
    else 
        echo "Invalid version format. Please try again."
    fi
done

echo "What has changed since last release?"
echo "Every change should be a new line starting with a dash. You can use (github) markdown. The indentation level of the section is two (##)."
CHANGES=$(</dev/stdin)
CHANGES_ESCAPED=$(echo "$CHANGES" | sed -z 's/\n/\\n/g')

echo "Checking gh cli auth status"
gh auth status

while [[ $? -eq 1 ]]
do
   gh auth login
   gh auth status
done

echo "Do you want to proceed with the release $VERSION (y/n)?"
read CONFIRM
if [ "$CONFIRM" != "y" ]; then
    echo "Aborting release"
    exit 1
fi

gh workflow run release.yml -f version="$VERSION" -f changes="$CHANGES_ESCAPED"