# winlogdecoder
Qt based application to decode windows log files(.etl and .evtx) to txt files.  
It is a multi-thread tool which means it can decode multiple files in parallel. The default number of threads is the logical processor(s) number of the CPU.  

## etl files
The purpose is to show the debug information from your windows drivers. The idea is like following:  
- Call the windows tool "tracerpt" to convert the .etl files to .xml files.
- Parse the .xml files, extract the information to .txt files.

Notes:
- The format of the generated txt file is like this:  
  [level]    timestamp    {guid of the provider}or[name of the provider][process id][thread id]    eventID    (Task Category)    message  
- If there is no message in the event, the event will not be shown in the txt file.  
- The message can be in the different sub element inside the "Event" element. The tool may not be able to cover all the cases. You may need modify the class EtlDecoder to cover the addtional cases.

## evtx files
The purpose is to show the windows events. The process is as following:
- Call the evtx_dump tool (https://github.com/omerbenamram/evtx) to convert the evtx files to xml files.
- The xml files converted by the evtx_dump tool can't be completely deocoded by the Qt class QXmlStreamReader, so I have to revise the format.
- Then parse the revised xml files and extract the information to .txt files.

Notes:
- The format of the generated txt file is like this:  
  [level]    timestamp    {guid of the provider}or[name of the provider][process id][thread id]    eventID    (Task Category)
- Currently, all events are printed to the txt file. You can modify the class EvtxDecoder to print the eventIDs you are interested only. 
