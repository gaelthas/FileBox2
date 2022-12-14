//
// dtuelink.js
// Copyright (C) 2003-2004 MGTEK. All rights reserved.
//

var ieVersion = GetIEVersion();
var jsPath = GetScriptPath();

main();

/////////////////////////////////////////////////////////////////////////////

// Determines the IE version. Returns 0 if not IE.
function GetIEVersion()
{
	if (navigator.appName == "Microsoft Internet Explorer")
	{
		var version = window.navigator.userAgent;
		var msie = version.indexOf("MSIE ");
		if (msie > 0)
			return parseFloat(version.substring(msie + 5, version.indexOf(";", msie)));
	}
	return 0;
}

// Determine path to script folder.
function GetScriptPath()
{
	var path = document.scripts[document.scripts.length - 1].src;
	return path.toLowerCase().replace("dtuelink.js", "");
}

function main()
{
	// Write the CSS and JScript links.
	if (ieVersion >= 5)
	{
		document.writeln('<LINK rel="stylesheet" href="ms-help://Hx/HxRuntime/HxLink.css">');
		document.writeln('<LINK rel="stylesheet" href="' + jsPath + "msdn.css" + '">');
		document.writeln('<SCRIPT src="' + jsPath + '\msdn_ie5.js"></SCRIPT>');
		document.writeln('<SCRIPT for="reftip" event="onclick">window.event.cancelBubble = true;</SCRIPT>');
		document.writeln('<SCRIPT for="cmd_lang" event="onclick">langClick(this);</SCRIPT>');
		document.writeln('<SCRIPT for="cmd_filter" event=onclick>filterClick(this);</SCRIPT>');
	}
	else
	{
		document.writeln('<LINK rel="stylesheet" href="' + jsPath + "msdn.css" + '">');
		document.writeln('<SCRIPT src="' + jsPath + '\msdn_ie4.js"></SCRIPT>');
	}
}
