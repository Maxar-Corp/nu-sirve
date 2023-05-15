backupfolder='backedup'

echo 'deleting previous backup folder'
rm -rf $backupfolder/

echo 'creating backup folder'
mkdir $backupfolder

echo 'backing up cpp files'
cppfiles=`find "SirveApp" -type f -name "*.cpp" -not -path "*/release/*"`
for cppfile in $cppfiles
do
	echo "$cppfile"
	cp $cppfile "$backupfolder/`basename $cppfile .cpp`.txt" 
done

echo 'backing up h files'
hfiles=`find "SirveApp" -type f -name "*.h" -not -path "*/release/*" -not -path "*ui_*"`
for hfile in $hfiles
do
	echo "$hfile"
	cp $hfile "$backupfolder/`basename $hfile .h`_h.txt"
done

echo 'backing up .pro file'
pro_file="SirveApp/SirveApp.pro"
echo "$pro_file"
cp $pro_file "$backupfolder/`basename $pro_file .pro`_pro.txt"

echo 'backing up .ui files'
uifiles=`ls -r SirveApp/*.ui`
for uifile in $uifiles
do
	echo "$uifile"
	cp $uifile "$backupfolder/`basename $uifile .ui`_ui.txt"
done

echo 'backing up icons'
cp -r SirveApp/icons "$backupfolder" 

echo 'backing up config folder'
cp -r SirveApp/config "$backupfolder"

echo 'backing up README'
cp README.md "$backupfolder"

echo 'done'
