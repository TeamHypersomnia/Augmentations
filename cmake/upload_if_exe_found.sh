#!/usr/bin/env bash 
EXE_PATH="build/current/Hypersomnia"

PLATFORM=$1
GIT_BRANCH=$2

if [ "$GIT_BRANCH" != "master" ]; then
	echo "Branch is $GIT_BRANCH. Skipping upload."
	exit 0
fi

if [ -f "$EXE_PATH" ]; then
	echo "Exe found. Uploading."

	COMMIT_HASH=$(git rev-parse HEAD)
	COMMIT_NUMBER=$(git rev-list --count master)
	COMMIT_MESSAGE=$(git log -1 --pretty=%B)
	VERSION="1.0.$COMMIT_NUMBER"
	FILE_PATH="Hypersomnia-for-$PLATFORM.sfx"
	UPLOAD_URL="https://hypersomnia.xyz/upload_artifact.php"

	. cmake/linux_launcher_install.sh

	if [[ "$PLATFORM" = "MacOS-updater" ]]
	then
		mv hypersomnia Hypersomnia.command
	fi

	cp build/current/Hypersomnia hypersomnia/.Hypersomnia
	pushd hypersomnia
	rm -r cache logs user
	popd
	7z a -sfx $FILE_PATH hypersomnia
	curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL

	if [[ "$PLATFORM" = "MacOS-updater" ]]
	then
		# Also upload a plain zip file for first-time downloads on MacOS

		PLATFORM="MacOS"
		FILE_PATH="Hypersomnia-for-$PLATFORM.zip"
		zip -r -sfx $FILE_PATH hypersomnia

		curl -F "key=$API_KEY" -F "platform=$PLATFORM" -F "commit_hash=$COMMIT_HASH" -F "version=$VERSION" -F "artifact=@$FILE_PATH" -F "commit_message=$COMMIT_MESSAGE" $UPLOAD_URL
	fi
else
	echo "No exe found. Not uploading."
fi
