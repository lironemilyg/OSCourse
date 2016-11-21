mkdir 305774382
cd 305774382
mkdir temp
echo liron > temp/liron
echo gazit > temp/gazit
echo lironemilyg > temp/lironemilyg
cp temp/liron .
cp temp/gazit .
rm temp/liron temp/gazit
mv temp/lironemilyg .
rm -r temp
echo "list of files in current dir"
ls -l
echo "first name file"
cat liron
echo "last name file"
cat gazit
echo "moodle username file"
cat lironemilyg