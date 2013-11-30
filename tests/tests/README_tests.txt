# README_UNITTESTS.TXT
# This file contains a brief description and documentation
# of the unit tests for the important functions for the Video CDN

*** Checkpoint 1: Bitrate Adaptation ***

ParseXML Test
	Goal: To test if the parseXML function properly finds and adds
				all of the available bitrates to an orderedList
	Procedure:
				1. Create a main function to test the function
						testParse.c
				2. Compile the parseTest and the orderedList file
						gcc ../orderedList/orderedList.c testParse.c -o parse
				3. Run the parseTest
						./parse
				4. Inspect the output
						The output is correct.
	Results:
		Found bitrate: 10
		Found bitrate: 100

		Printing out bitrates in list from file
		listSize: 2
		bitrate: 10
		bitrate: 100

	Known Issues: None

Logger Test
	Goal: To test if the log functions properly log the desired
				output in the correct format
	Procedure:
			1. Create a main function to test the function
					log.c
			2. Compile the logTest and the log file
					gcc ../log/log.c testLog.c -o log
			3. Run the logTest
					./log
			4. Inspect the output
					The output is correct. The logger correctly logs the time since
					the epoch in seconds.
	Results:
	140734281437256 Test 1
	140734281437256 Test 2
	140734281437224 Closing log file

	Known Issues: None


*** Checkpoint 2: Final Submission ***
