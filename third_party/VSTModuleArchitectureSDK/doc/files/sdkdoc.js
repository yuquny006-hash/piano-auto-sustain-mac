function popupGroup (name)
{
	if(document.getElementById) // IE and Netscape >= 6
	{
		obj = document.getElementById (name); 
		if(obj)
		{
			if(obj.style.display == "")
				obj.style.display = "none";
			else
				obj.style.display = "";
		}
	}
	else if(document.all) // IE
	{
		obj = document.all[name];	
		if(obj)
		{
			if(obj.style.display == "")
				obj.style.display = "none";
			else
				obj.style.display = "";
		}
	}
	else if(document.layers) // Netscape
	{
		if(document.layers[name].visibility == "show")
			document.layers[name].visibility = "hide";
		else
			document.layers[name].visibility = "show";			
	}
	//else
	//	alert ("Browser not compatible!");
}
