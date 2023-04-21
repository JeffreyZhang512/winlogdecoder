# winlogdecoder
Qt based application to decode windows log files(.etl) to txt files

The idea is like following:
- Call the windows tool "tracerpt" to convert the .etl files to .xml files.
- Parse the .xml files, extract the information to .txt files.

Notes:
- The format of the generated txt file is like this:
  timestamp    {name or guid of the provider}[process id][thread id]    message
- The message can be in the different sub element inside the "Event" element. So, the tool may not cover all the cases. Users need modify the class EtlDecoder to cover the addtional cases.
- The tool is muti-thread which means it can decode multiple .etl files in parallel. The default number of threads is the logical processor(s) number of the CPU. 
