#!/bin/bash
cd "$(dirname "$0")"
if [ $1 ]; then
  pattern='^[0-9]{1,2}+([.][0-9]{1,2}+([.][0-9]{1,2}))?$'
  if ! [[ $1 =~ $pattern ]]; then
    echo "Specify version number (major.minor.patch)"
    echo $1
    exit 1
  fi

  echo "Building revengeMusic_$1.deb..."

else
  echo "Specify version number (major.minor.patch)"
  exit 1
fi

#Make directories and copy files
mkdir -p "revengeMusic_$1"
mkdir -p "revengeMusic_$1/usr/"
mkdir -p "revengeMusic_$1/usr/bin/"
mkdir -p "revengeMusic_$1/DEBIAN/"
cp "control" "revengeMusic_$1/DEBIAN/"
cp "../../build/revengeMusic" "revengeMusic_$1/usr/bin/revengemusic"

#Create symbolic links
cd "revengeMusic_$1/usr/bin/"
ln -s "revengemusic" "revengeMusic"
ln -s "revengemusic" "rmusic"
cd "../../../"

#Make deb
dpkg-deb --build "revengeMusic_$1"

echo -e "\n"
read -p "Delete directories used to build revengeMusic_$1? [y/N]" -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    exit 1
fi

rm -r "revengeMusic_$1/"
