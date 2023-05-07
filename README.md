# winlogdecoder
This is a Qt based tool to decode Windows log files(.etl and .evtx) to txt files. It is a multi-thread tool which means it can decode multiple files in parallel. The default number of threads is the logical processor(s) number of the CPU.  

For the binary release, please goto:  
https://github.com/JeffreyZhang512/winlogdecoder/releases  
The latest release version is v0.5.0.0 which also includes the revised evtx_dump tool.  
To run the binary, you need install the Microsoft Visual C++ Redistributable as well. You can download it from Microsoft website.

## etl files
The purpose of decoding etl files is to show the debug information printed from your windows drivers. The tool will be helpful if you are a Windows driver developer. The decoding process is like following:  
- Call the windows tool "tracerpt" to convert the .etl files to .xml files.
- Parse the .xml files, extract the debug information to .txt files.

Notes:
- The format of the generated txt file is like this:  
  [level]    timestamp    {guid of the provider}or[name of the provider][process id][thread id]    eventID    (Task Category)    message  
- If there is no debug information in the event, the event will not be shown in the txt file.  
- The debug information can be in the different sub element inside the "Event" element. The tool may not be able to cover all the cases. You may need modify the class EtlDecoder to cover the addtional cases.

## evtx files
The purpose of decoding evtx files is to show the events recorded by Windows. The process is as following:
- Call the evtx_dump tool (https://github.com/omerbenamram/evtx) to convert the evtx files to xml files.
- Then parse the revised xml files and extract the information to .txt files.
- The problem is that the output of the evtx_dump tool is in XML format but not a valid xml file which can't be completely parsed by the Qt class QXmlStreamReader, so I have to change the evtx_dump tool to make the output file as a valid xml file.

Notes:
- The format of the generated txt file is like this:  
  [level]    timestamp    {guid of the provider}or[name of the provider][process id][thread id]    eventID    (Task Category)
- Currently, all events are printed to the txt file. You can modify the class EvtxDecoder to print the eventIDs you are interested only.
