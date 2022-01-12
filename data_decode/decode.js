function ProtocolToRaw(obj) {
	var data = new Array();
	var data_offset = 0;
    data[data_offset++]= 5;// fport=5
    data[data_offset++]= 0;// unconfirmed mode

    if(("open_screen_text" in obj.params))
    {

        data[2] = 1; //open_screen_text
        data[3] = obj.params.open_screen_text.length; //字符串长度
        for(var i=0;i<obj.params.open_screen_text.length;i++)
        {
            data[4+i] = obj.params.open_screen_text[i].charCodeAt();
        }
    }
	else
	{
		if(("company" in obj.params))
		{
			data[data_offset++] = 2; //name
			data[data_offset++] = obj.params.company.length/2; //字符串长度
			for(var i=0;i<obj.params.company.length;)
			{
				if(obj.params.company[i].charCodeAt()	< 65)
				{
					data[data_offset+i/2] = ((obj.params.company[i].charCodeAt()- 48)<<4);
				}
				else
				{
					data[data_offset+i/2] = ((obj.params.company[i].charCodeAt()- 65 +10)<<4);
				}
	
				if(obj.params.company[i+1].charCodeAt()  < 65)
				{
					data[data_offset+i/2] += (obj.params.company[i+1].charCodeAt()- 48);
				}
				else
				{
					data[data_offset+i/2] += (obj.params.company[i+1].charCodeAt()- 65 +10);
				}
				i+=2;
			}
			data_offset += obj.params.company.length/2;
		}  
		if(("title" in obj.params))
		{
			data[data_offset++] = 3; //name
			data[data_offset++] = obj.params.title.length/2; //字符串长度
			for(var i=0;i<obj.params.title.length;)
			{
				if(obj.params.title[i].charCodeAt()	< 65)
				{
					data[data_offset+i/2] = ((obj.params.title[i].charCodeAt()- 48)<<4);
				}
				else
				{
					data[data_offset+i/2] = ((obj.params.title[i].charCodeAt()- 65 +10)<<4);
				}
	
				if(obj.params.title[i+1].charCodeAt()  < 65)
				{
					data[data_offset+i/2] += (obj.params.title[i+1].charCodeAt()- 48);
				}
				else
				{
					data[data_offset+i/2] += (obj.params.title[i+1].charCodeAt()- 65 +10);
				}
				i+=2;
			}
			data_offset += obj.params.title.length/2;
		} 

		if(("name" in obj.params))
		{
			data[data_offset++] = 4; //name
			data[data_offset++] = obj.params.name.length/2; //字符串长度
			for(var i=0;i<obj.params.name.length;)
			{
				if(obj.params.name[i].charCodeAt()	< 65)
				{
					data[data_offset+i/2] = ((obj.params.name[i].charCodeAt()- 48)<<4);
				}
				else
				{
					data[data_offset+i/2] = ((obj.params.name[i].charCodeAt()- 65 +10)<<4);
				}
	
				if(obj.params.name[i+1].charCodeAt()  < 65)
				{
					data[data_offset+i/2] += (obj.params.name[i+1].charCodeAt()- 48);
				}
				else
				{
					data[data_offset+i/2] += (obj.params.name[i+1].charCodeAt()- 65 +10);
				}
				i+=2;
			}
			data_offset += obj.params.name.length/2;
		} 
	}
    
   return data;
}
