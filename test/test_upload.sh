URL="http://localhost:8080/uploads"
FILE1="test_file1.txt"
FILE2="test_file2.txt"

upload_file() {
	local file="$1"

	if [[ ! -f "$file" ]]; then
		echo "[$file] file not found"
		return 1
	fi

	echo "[$file] starting upload"

	http_code=$(curl -s -w "%{http_code}" \
		-X POST "$URL" \
		-F "file=@${file}")

	echo "[$file] upload completed with HTTP code: $http_code"
}

upload_file "$FILE1" &
PID1=$!
 
upload_file "$FILE2" &
PID2=$!
 
wait $PID1
wait $PID2
 
echo "all processes completed"
