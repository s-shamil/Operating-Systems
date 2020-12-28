find_in_csv()
{
    #assumes working in ONE
	cat CSE_322.csv | while read line
	do 
		name=$(echo "$line" | cut -d',' -f2)
		#echo $name
		idstray=$(echo "$line" | cut -d',' -f1)
		#echo $idstray
		idspace=$(echo "$idstray" | sed 's/"//g')
		#echo $idspace
		id=$(echo "$idspace" | sed 's/\ //g')
		#echo $id
		s="$1"
		#flag=0
		#for subs in $*
		#do
		#	if [ "$flag" = "1" ]; then
		#		s=" $s $subs"
		#	fi
		#	flag=1
		#done
		s1=$(echo ${name^^})
		s2=$(echo ${s^^})
		if [ "$s1" = "$s2" ]; then
			echo $id
		fi
	done
}
intersect_csv_abs()
{
    #assumes working in ONE
	find_in_csv "$1" | while read roll
	do
		grep $roll WorkingDirectory/AbsenteeList.txt
	done
}


if [ -d WorkingDirectory ]; then
	rm -rf WorkingDirectory
fi 
mkdir WorkingDirectory
cp SubmissionsAll.zip WorkingDirectory
cp CSE_322.csv WorkingDirectory
cd WorkingDirectory
unzip -q SubmissionsAll.zip

if [ -f AbsenteeList.txt ]; then
	rm -v AbsenteeList.txt
fi
cat CSE_322.csv | while read line
do
	curr=$(echo $(echo $line | sed 's/[^0-9]*//g'))
	#echo -n $curr " "
	cnt=$(find . -name "*${curr}.zip" | wc -l)
	if [ $cnt = "0" ]; then
		echo $curr >> AbsenteeList.txt
	fi
done
rm -v SubmissionsAll.zip
rm -v CSE_322.csv

if [ -d Output ]; then
	rm -rf Output
fi
mkdir Output
cd Output
mkdir Extra
cd ..



if [ -f Marks.txt ]; then
    rm -v Marks.txt
fi


find . -name "*.zip" | while read zipname
do
    filename=$(echo "$zipname" | cut -d'/' -f2)
    front=$(echo "$filename" | sed 's/\.zip//')
    echo "$front"
    name=$(echo "$front" | cut -d'_' -f1)
    stdid=$(echo "$front" | cut -d'_' -f5)
    echo "$name $stdid"
    
    if [ -d temp ]; then
        rm -rf temp
    fi
    mkdir temp

    cp "$zipname" temp
    cd temp
    unzip -q "$zipname"
    rm "$zipname"
    
    ls
    
    cnt=$(echo $(ls | wc -l))
    if [ $cnt = "1" ]; then
        #one subdir only
        echo -n "<one subdir> "
        case $stdid in
            [0-9][0-9][0-9][0-9][0-9][0-9][0-9] )
                #valid student id
				echo -n "<valid ID found in zip name> "
                case $(ls) in
                    $stdid ) 
						echo "<perfect naming> ";
                        cd ..;
                        echo "$stdid" " 10">>Marks.txt;
                        cd temp;
                        pushd .. > /dev/null;
                        mv "temp/${stdid}" "Output";
                        popd > /dev/null;;

                    *${stdid}* ) 
                        echo "<name consists id plus stray> ";
                        cd ..;
                        echo "$stdid" "  5">>Marks.txt;
                        cd temp;
                        mv "$(ls)" "$stdid";
                        pushd .. > /dev/null;
                        mv "temp/${stdid}" "Output";
                        popd > /dev/null;;

                    * ) 
                        echo "<folder name complete wrong>";
                        cd ..;
                        echo "$stdid" "  0">>Marks.txt;
                        cd temp;
                        mv "$(ls)" "$stdid";
                        pushd .. > /dev/null;
                        mv "temp/${stdid}" "Output";
                        popd > /dev/null;;
						
                esac
                ;;
            * ) 
                echo "<id not found in zip name>";
                cd ..; #temp->workingdir;
                cd ..; #workingdir->ONE;
                count_match=$(intersect_csv_abs "$name" | wc -l);
                echo $count_match;
                if [ "$count_match" = "1" ]; then
                    #tracked down - goes to output - gets 0 marks
                    tracked_id=$(intersect_csv_abs "$name");
                    cd WorkingDirectory;
                    echo "$tracked_id" "  0">>Marks.txt;
                    sed -i "/${tracked_id}/d" AbsenteeList.txt;
                    
                    cd temp;
                    mv "$(ls)" "dummyname"; #if name given by student matches the tracked id, dummyname used as transition state
                    mv "dummyname" "$tracked_id";
                    cd ..;
                    mv "temp/${tracked_id}" "Output";
                    cd temp;
                else
                    #cant track - move to extra with name as folder name
                    cd WorkingDirectory/temp;
                    mv "$(ls)" "dummyname"; #if name given by student matches the tracked id, dummyname used as transition state
                    mv "dummyname" "$name";
                    cd ..;
                    mv "temp/${name}" "Output/Extra";
                    cd temp;
                fi
        esac
    else
        echo "<multiple directories found>"
        #moving all folders to student named folder
        #temp folder e achi
        if [ -d "$name" ]; then
            rm -rf "$name"
        fi
        mkdir "$name"
        ls | while read dir
        do
            if [ -d "$dir" ]; then
                if [ "$dir" != "$name" ]; then
                    mv "$dir" "$name";
                fi 
            fi
        done
        #cd ..;
        #mv "temp/${name}" "Output/Extra";
        
        case $stdid in
            [0-9][0-9][0-9][0-9][0-9][0-9][0-9] )
                #valid student id in zip name
                echo "<zip name contains id>"
                mv "$name" "$stdid";
                cd ..;
                echo "$stdid" "  0">>Marks.txt;
                mv "temp/${stdid}" "Output";
                cd temp;;
            *)
                #trying to identify
                cd ..; #temp->workingdir;
                cd ..; #workingdir->ONE;
                count_match=$(intersect_csv_abs "$name" | wc -l);
                echo $count_match;
                if [ "$count_match" = "1" ]; then
                    #tracked down - goes to output - gets 0 marks
                    tracked_id=$(intersect_csv_abs "$name");
                    cd WorkingDirectory;
                    echo "$tracked_id" "  0">>Marks.txt;
                    sed -i "/${tracked_id}/d" AbsenteeList.txt;
                    
                    cd temp;
                    mv "$name" "$tracked_id";
                    cd ..;
                    mv "temp/${tracked_id}" "Output";
                    cd temp;
                else
                    #cant track - move to extra with name as folder name
                    cd WorkingDirectory;
                    mv "temp/${name}" "Output/Extra";
                    cd temp;
                fi
                ;;
        esac
        
        #cd temp; 
    fi

    echo "--------------------"
    cd ..
done

if [ -d temp ]; then
    rm -rf temp
fi

cat AbsenteeList.txt | while read rollnum
do
    echo "$rollnum" "  0">>Marks.txt;
done

sort Marks.txt >> Marks_temp.txt;
cp Marks_temp.txt Marks.txt;
rm Marks_temp.txt;

ls | while read compressed_file
do
    case "$compressed_file" in 
        *.zip | *.rar )
            rm "$compressed_file";;
        * )
        ;;
    esac
done    