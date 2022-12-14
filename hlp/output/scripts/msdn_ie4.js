//
// msdn_ie4.js
// Copyright (C) 2003-2004 MGTEK. All rights reserved.
//

/////////////////////////////////////////////////////////////////////////////
// Localization strings

// Strings for expand-collapse functions
var L_ExpandAll_TEXT = "Expand All";
var L_CollapseAll_TEXT = "Collapse All";
var L_ExColl_TEXT = "Click to Expand or Collapse";

// End localization
/////////////////////////////////////////////////////////////////////////////

window.onload = main;

function main()
{
}

/////////////////////////////////////////////////////////////////////////////
// Expandable/Collapsable sections
/////////////////////////////////////////////////////////////////////////////

function makeExpandable(title, level)
{
	if (title != "")
	{
		var code = '<A href="#" id="expand" class="expandLink' + level + '" onclick="ToggleSection()">';
		code += '<IMG class="expand" src="' + jsPath + 'expand.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + title;
		code += '</A>';
		code += '<BR><DIV class="expandBody">';
		document.write(code);
	}
	else
	{
		var code = '<A href="#" id="expandall" onclick="ExpandAllSections()" class="expandLink' + level + '">';
		code += '<IMG class="expandall" src="' + jsPath + 'expand.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + L_ExpandAll_TEXT;
		code += '</A>';
		document.write(code);
	}
}

function GetExpandCollapseLink(e)
{
	while (e.tagName != "A")
		e = e.parentElement;
	return e;
}

function GetExpandCollapseImage(e)
{
	return e.all.tags("IMG")(0);
}

function GetExpandCollapseDiv(e)
{
	while (e)
	{
		if (e.tagName == "DIV" && e.className.toLowerCase() == "expandbody")
			return e;
		e = GetNextSibling(e);
	}
	return null;
}

function ToggleSection()
{
	event.returnValue = false;

	var link = GetExpandCollapseLink(window.event.srcElement);
	var div = GetExpandCollapseDiv(link);
	var img = GetExpandCollapseImage(link);

	if (div.style.display == "block")
	{
		img.src = jsPath + "expand.gif";
		div.style.display = "none";
	}
	else
	{
		img.src = jsPath + "collapse.gif";
		div.style.display = "block";
	}
}

function ExpandAllSections()
{
	event.returnValue = false;

	var link = GetExpandCollapseLink(window.event.srcElement);

	var expandAll = link.innerHTML.indexOf(L_ExpandAll_TEXT) != -1;
	if (expandAll)
	{
		var code = '<IMG class="expandall" src="' + jsPath + 'collapse.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + L_CollapseAll_TEXT;
		link.innerHTML = code;
	}
	else
	{
		var code = '<IMG class="expandall" src="' + jsPath + 'expand.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + L_ExpandAll_TEXT;
		link.innerHTML = code;
	}

	var links = document.links;
	for (var i = 0; i < links.length; i++)
	{
		var link = links(i);

		if (link.id == "expand")
		{
			var div = GetExpandCollapseDiv(link);
			var img = GetExpandCollapseImage(link);
			if (expandAll)
			{
				img.src = jsPath + "collapse.gif";
				div.style.display = "block";
			}
			else
			{
				img.src = jsPath + "expand.gif";
				div.style.display = "none";
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Feedback links
/////////////////////////////////////////////////////////////////////////////

function sendfeedback(id, alias)
{
	var url = location.href;
	var title = document.all.tags("TITLE")(0).innerText;
	var browser = navigator.appName + " " + navigator.appVersion
	var href = "mailto:" + alias + "?subject=" + id + "%20" + title + "&body=Topic%20ID:%20" + id + "%0d%0a";
	href += "Topic%20Title:%20" + title + "%0d%0a";
	href += "URL:%20" + url + "%0d%0a";
	href += "Browser:%20" + browser + "%0d%0a%0d%0a";
	href += "Comments:%20";
	location.href = href;
}

/////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////

function GetNextSibling(e)
{
    return document.all(e.sourceIndex + e.children.length + 1);
}
