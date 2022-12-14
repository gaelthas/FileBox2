//
// msdn_ie5.js
// Copyright (C) 2003-2004 MGTEK. All rights reserved.
//

/////////////////////////////////////////////////////////////////////////////
// Localization strings

// Strings for expand-collapse functions
var L_ExpandAll_TEXT = "Expand All";
var L_CollapseAll_TEXT = "Collapse All";
var L_ExColl_TEXT = "Click to Expand or Collapse";

// Strings for popup sections
var L_SeeAlso_TEXT = "See Also";
var L_Requirements_TEXT = "Requirements";

// Strings for language filtering
var L_FilterTip_TEXT = "Language Filter";
var L_Language_TEXT = "Language";
var L_ShowAll_TEXT = "Show All";

// Glossary popups
var L_PopupLinkTitle = "Glossary Term";

// End localization
/////////////////////////////////////////////////////////////////////////////

var g_DropShadows = new Array;
var g_DropShadowDepth = 4;
var g_DropShadowColor = "gray";

/////////////////////////////////////////////////////////////////////////////

window.onload = main;

function main()
{
	InitializeGlossaryPopup();
	InitializePopupMenu("seealso", L_SeeAlso_TEXT);
	InitializePopupMenu("requirements", L_Requirements_TEXT);
	InitializeLanguages();
	InitializeReftips();

	window.onresize = OnBodyResize;
	document.onkeypress = OnBodyKeyPress;
	document.body.onclick = OnBodyClick;

	ResetAllButtons();

	ResizeBanner();
}

function OnBodyResize()
{
	ResizeBanner();
	ResetGUI();
}

function OnBodyKeyPress()
{
	if (window.event.keyCode == 27)	// Escape key
		ResetGUI();
}

function OnBodyClick()
{
	ResetGUI();
}

function ResetGUI()
{
	HideTip();
	HideGlossaryPopup();
	HideAllPopupMenus();
	ResetAllButtons();
}

/////////////////////////////////////////////////////////////////////////////
// Non-scrolling Banner
/////////////////////////////////////////////////////////////////////////////

function ResizeBanner()
{
	if (document.body.clientWidth == 0)
		return;

	var nsbanner = document.all.nsbanner;
	var nstext = document.all.nstext;
	if (!nsbanner || !nstext)
		return;

	document.body.scroll = "no"
 	nsbanner.style.width = document.body.offsetWidth - 2;
	nstext.style.overflow = "auto";
	nstext.style.width = document.body.offsetWidth - 4;
   	nstext.style.height = Math.max(document.body.offsetHeight - (nsbanner.offsetHeight + 4), 0);

	try
	{
		nstext.setActive();
	}
	catch(e)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// Common popup menu code
/////////////////////////////////////////////////////////////////////////////

var g_PopupMenus = new Array;

function InsertButtonAndPopupMenu(id, name, div)
{
	var id = id;
	var td = hdr.insertCell(g_PopupMenus.length);
	if (td)
	{
		td.className = "button";
		td.onclick = ShowPopupMenu;
		td.onkeypress = ShowPopupMenu;
		td.onkeyup = OnBodyKeyPress;
		td.innerHTML = '<IMG id="button" popupid="' + id + '" src="' + jsPath + id + '1.gif' + '" alt="' + name + '" border=0 tabindex=0>';
		nsbanner.insertAdjacentHTML('afterEnd', div);
		g_PopupMenus[g_PopupMenus.length] = id;
	}
}

function ShowPopupMenu()
{
	window.event.returnValue = false;
	window.event.cancelBubble = true;

	ResetGUI();

	var source = window.event.srcElement;

	var id = source.popupid;
	if (id)
	{
		source.src = source.src.replace("1.gif", "2.gif");

		var div = document.all[id];
		if (div)
		{
			div.style.pixelTop = source.offsetTop + source.offsetHeight;
			div.style.visibility = "visible";
			CreateDropShadow(div);
		}
	}
}

function HideAllPopupMenus()
{
	for (var i = 0; i < g_PopupMenus.length; i++)
	{
		var id = g_PopupMenus[i];
		var div = document.all.item(id);
		if (div)
		{
			div.style.visibility = "hidden";
			RemoveDropShadow();
		}
	}
}

function ResetAllButtons()
{
	var buttons = document.all.button;
	if (buttons)
	{
		if (buttons.src)
		{
			buttons.src = buttons.src.replace("2.gif", "1.gif");
		}
		else
		{
			for (var i = 0; i < buttons.length; i++)
				buttons[i].src = buttons[i].src.replace("2.gif", "1.gif");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// "See Also" and "Requirements" Popup Menus
/////////////////////////////////////////////////////////////////////////////

function InitializePopupMenu(id, name)
{
	var hdr = document.all.hdr;
	if (!hdr)
		return;

	var text = "";

	// Get the content for the popup window.
	var headings = document.all.tags("H4");
	for (var i = 0; i < headings.length; i++)
	{
		var heading = headings[i];

		// Find H4 elements that match 'name'.
		if (BeginsWith(heading.innerText, name))
		{
			// Found one. Get elements until we hit the next heading element.
			text += heading.outerHTML;
			var e = heading.nextSibling;
			while (e && !e.tagName.match(/^(H[1-4])|(DIV)$/))
			{
				text += e.outerHTML;
				e = e.nextSibling;
			}
		}
	}

	// Create the popup DIV.
	if (text != "")
	{
		var div = '<DIV id="' + id + '" class="popup" onkeypress="OnBodyKeyPress">' + text + '</DIV>';
		InsertButtonAndPopupMenu(id, name, div);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Glossary Popups
/////////////////////////////////////////////////////////////////////////////

var g_PopupStyle = "width: 50%; padding: 5px 8px; background: #ffffcc; border: solid 1px #999999;";
var g_PopupLinkStyle = "cursor: hand; color: green; text-decoration: underline;";

function InitializeGlossaryPopup()
{
	if (document.all.glossarypopuplink)
	{
		var html = '';

		html += '<OBJECT id="oGlossary" classid="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83" style="display:none; position:absolute;">\n';
		html += '\t<PARAM name="DataURL" value="glossary.csv">\n';
		html += '\t<PARAM name="UseHeader" value="True">\n';
		html += '\t<PARAM name="TextQualifier" value="|">\n';
		html += '</OBJECT>\n';

		html += '<SCR' + 'IPT language="javascript" for="glossarypopuplink" event="onmouseover">\n';
		html += '\tthis.title = "' + L_PopupLinkTitle + '";';
		html += '</SCR' + 'IPT>\n';
		html += '<SCR' + 'IPT language="javascript" for="glossarypopuplink" event="onclick">\n';
		html += '\tShowGlossaryPopup();\n';
		html += '</SCR' + 'IPT>\n';

		html += '<DIV id="glossarypopup" class="glossarypopup"></DIV>\n';

		nsbanner.insertAdjacentHTML("afterEnd", html);
	}
}

function ShowGlossaryPopup()
{
	window.event.returnValue = false;
	window.event.cancelBubble = true;
	
	ResetGUI();

	var div = document.all.glossarypopup;
	if (div)
	{
		var e = window.event.srcElement;
		var index = e.href.lastIndexOf("#");
		if (index >= 0)
		{
			var key = e.href.substr(index + 1);
			var item = GetGlossaryItem(key);
			if (item.Found)
			{
				div.innerHTML = '<H4>' + item.Term + '</H4><P>' + item.Definition + '</P>';
				var bodyWidth = document.body.clientWidth - g_DropShadowDepth;
				var bodyHeight = document.body.clientHeight - g_DropShadowDepth;
				var x = window.event.clientX;
				var y = window.event.clientY;
				var width = div.offsetWidth;
				var height = div.offsetHeight;
				if (x + width > bodyWidth && width < bodyWidth)
					x = bodyWidth - width;
				if (y + height > bodyHeight && height < bodyHeight)
					y = bodyHeight - height;
				div.style.left = x + document.body.scrollLeft;
				div.style.top = y + document.body.scrollTop;
				div.style.visibility = "visible";
				CreateDropShadow(div);
			}
		}
	}
}

function HideGlossaryPopup()
{
	var div = document.all.glossarypopup;
	if (div)
	{
		div.style.visibility = "hidden";
		RemoveDropShadow();
	}
}

function GetGlossaryItem(key)
{
	var item = new GlossaryItem();
	oGlossary.Filter = "key=" + key;
	oGlossary.Reset();
	
	var recordset = oGlossary.recordset;
	if (recordset.RecordCount > 0)
	{
		recordset.MoveFirst();
		item.Term = recordset.fields.item("term").value;
		item.Definition = recordset.fields.item("definition").value;
		item.Found = true;
	}
	return item;
}

function GlossaryItem()
{
	this.Term = "";
	this.Definition = "";
	this.Found = false;
}

/////////////////////////////////////////////////////////////////////////////
// Expandable/Collapsable sections
/////////////////////////////////////////////////////////////////////////////

function makeExpandable(title, level)
{
	if (title != "")
	{
		var code = '<A href="#" id="expand" class="expandlink' + level + '" onclick="ToggleSection()">';
		code += '<IMG class="expand" src="' + jsPath + 'expand.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + title;
		code += '</A>';
		code += '<BR><DIV class="expandbody">';
		document.write(code);
	}
	else
	{
		var code = '<A href="#" id="expandall" onclick="ExpandAllSections()" class="expandlink' + level + '">';
		code += '<IMG class="expandall" src="' + jsPath + 'expand.gif" width="9" height="9" border="0" alt="' + L_ExColl_TEXT + '">';
		code += '&nbsp;' + L_ExpandAll_TEXT;
		code += '</A>';
		document.write(code);
	}
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
		e = e.nextSibling;
	}
	return null;
}

/////////////////////////////////////////////////////////////////////////////
// Language filtering
/////////////////////////////////////////////////////////////////////////////

function InitializeLanguages()
{
	var hdr = document.all.hdr;
	if (!hdr)
		return;

	var languages = EnumerateLanguages();
	if (languages.length == 0)
		return;

	var language = DetermineLanguage();
	if (language == null)
		language = GetLanguageFromCookie(languages);

	var pres = document.all.tags("PRE");
	if (pres)
	{
		for (var i = 0; i < pres.length; i++)
			InitializePreElement(pres[i]);
	}

	// Create language indicator.
	var head = document.all.tags("H1")[0];
	head.insertAdjacentHTML('beforeEnd', '<SPAN class="ilang"></SPAN>');

	var id = "language";
	var div = '<DIV id="' + id + '" class="language" onkeypress="OnBodyKeyPress"><B>' + L_Language_TEXT + '</B><UL>';
	for (var i = 0; i < languages.length; i++)
		div += '<LI><A href="#" onclick="SelectLanguage(this)">' + languages[i] + '</A><BR>';
	div += '<LI><A href="" onclick="SelectAllLanguages()">' + L_ShowAll_TEXT + '</A></UL></DIV>';
	InsertButtonAndPopupMenu(id, L_FilterTip_TEXT, div);

	if (language != null)
		FilterLanguages();
}

function SelectLanguage(item)
{
	window.event.returnValue = false;
	window.event.cancelBubble = true;

	ResetGUI();

	if (item)
	{
		var language = item.innerText;
		StoreLanguageInCookie(language);
		FilterLanguages(language);
	}
}

function SelectAllLanguages()
{
	window.event.returnValue = false;
	window.event.cancelBubble = true;

	ResetGUI();

	StoreLanguageInCookie(null);
	UnfilterLanguages();
}

function FilterLanguages(language)
{
	var spans = document.all.tags("SPAN");
	for (var i = 0; i < spans.length; i++)
	{
		var e = spans[i];
		if (e.className == "lang")
		{
			var newVal = FilterMatch(e.innerText, language) ? "block" : "none";
			var block = GetBlockElement(e);
			block.style.display = newVal;
			e.style.display = "none";

			if (block.tagName == "DT")
			{
				var next = block.nextSibling;
				if (next && next.tagName == "DD")
					next.style.display = newVal;
			}
			else if (block.tagName == "DIV")
			{
				block.className = "filtered2";
			}
			else if (block.tagName.match(/^H[1-6]$/))
			{
				if (IsTopicHeading(block))
				{
					if (newVal != "none")
					{
						var tag = null;
						if (block.children && block.children.length)
						{
							tag = block.children[block.children.length - 1];
							if (tag.className == "ilang")
							{
								tag.innerHTML = (newVal == "block") ?
									'&nbsp; [' + language + ']' : "";
							}
						}
					}
				}
				else
				{
					var next = block.nextSibling;
					while (next && !next.tagName.match(/^(H[1-6])$/))
					{
						if (next.tagName =="DIV")
						{
							if (next.className.toUpperCase() != "TABLEDIV") break;
						}
						next.style.display = newVal;
						next = next.nextSibling;
					}
				}
			}
		}
		else if (e.className == "ilang")
		{
			var block = GetBlockElement(e);
			if (block.tagName == "H1")
				e.innerHTML = '&nbsp; [' + language + ']';
		}
	}
	ResizeBanner();
}

function UnfilterLanguages()
{
	var spans = document.all.tags("SPAN");
	for (var i = 0; i < spans.length; i++)
	{
		var e = spans[i];
		if (e.className == "lang")
		{
			var block = GetBlockElement(e);
			block.style.display = "block";
			e.style.display = "inline";

			if (block.tagName == "DT") {
				var next = block.nextSibling;
				if (next && next.tagName == "DD")
					next.style.display = "block";
			}
			else if (block.tagName == "DIV")
			{
				block.className = "filtered";
			}
			else if (block.tagName.match(/^H[1-6]$/))
			{
				if (IsTopicHeading(block))
				{
					var tag = null;
					if (block.children && block.children.length)
					{
						tag = block.children[block.children.length - 1];
						if (tag && tag.className == "ilang")
							tag.innerHTML = "";
					}
				}
				else
				{
					var next = block.nextSibling;
					while (next && !next.tagName.match(/^(H[1-6])$/))
					{
						if (next.tagName =="DIV")
						{
							if (next.className.toUpperCase() != "TABLEDIV")
								break;
						}
						next.style.display = "block";
						next = next.nextSibling;
					}
				}
			}
		}
		else if (e.className == "ilang")
		{
			e.innerHTML = "";
		}
	}
	ResizeBanner();
}

function InitializePreElement(pre)
{
	var htm0 = pre.outerHTML;

	var reLang = /<span\b[^>]*class="?lang"?[^>]*>/i;
	var first = -1;
	var second = -1;

	first = htm0.search(reLang);
	if (first >= 0)
	{
		iPos = first + 17;
		iMatch = htm0.substr(iPos).search(reLang);
		if (iMatch >= 0)
			second = iPos + iMatch;
	}

	if (second < 0)
	{
		var htm1 = TrimPreElement(htm0);
		if (htm1 != htm0)
		{
			pre.insertAdjacentHTML('afterEnd', htm1);
			pre.outerHTML = "";
		}
	}
	else
	{
		var rePairs = /<(\w+)\b[^>]*><\/\1>/gi;

		var substr1 = htm0.substring(0, second);
		var tags1 = substr1.replace(/>[^<>]+(<|$)/g, ">$1");
		var open1 = tags1.replace(rePairs, "");
		open1 = open1.replace(rePairs, "");

		var substr2 = htm0.substring(second);
		var tags2 = substr2.replace(/>[^<>]+</g, "><");
		var open2 = tags2.replace(rePairs, "");
		open2 = open2.replace(rePairs, "");

		pre.insertAdjacentHTML('afterEnd', open1 + substr2);
		pre.insertAdjacentHTML('afterEnd', TrimPreElement(substr1 + open2));
		pre.outerHTML = "";
	}
}

function TrimPreElement(html)
{
	return html.replace(/[ \r\n]*((<\/[BI]>)*)[ \r\n]*<\/PRE>/g, "$1</PRE>").replace(/\w*<\/SPAN>\w*((<[BI]>)*)\r\n/g, "\r\n</SPAN>$1");
}

function GetBlockElement(e)
{
	while (e && e.tagName.match(/^([BIUA]|(SPAN)|(CODE)|(TD))$/))
		e = e.parentElement;
	return e;
}

function IsTopicHeading(head)
{
	for (var i = 0; i < nstext.children.length; i++)
	{
		var e = nstext.children[i];
		if (e.sourceIndex < head.sourceIndex)
		{
			if (e.tagName.match(/^(P)|(PRE)|([DOU]L)$/))
				return false;
		}
		else
		{
			break;
		}
	}
	return true;
}

function FilterMatch(text, name)
{
	var a = text.split(",");
	for (var iTok = 0; iTok < a.length; iTok++)
	{
		var m = a[iTok].match(/([A-Za-z].*[A-Za-z+#0-9])/);
		if (m && m[1] == name)
			return true;
	}
	return false;
}

function GetLanguageFromCookie(languages)
{
	var obj = document.all.obj_cook;
	if (obj && obj.object)
	{
		if (obj.getValue("lang.all") != "1")
		{
			var lang = obj.getValue("lang");
			var c = languages.length;
			for (var i = 0; i != c; ++i)
			{
				if (languages[i] == lang)
					return lang;
			}
		}
	}
	return null;
}

function StoreLanguageInCookie(language)
{
	var obj = document.all.obj_cook;
	if (obj && obj.object)
	{
		if (language == null)
		{
			obj.putValue('lang.all', '1');
		}
		else
		{
			obj.putValue('lang', language);
			obj.putValue('lang.all', '');
		}
	}
}

function EnumerateLanguages()
{
	var languages = new Array;

	var spans = document.all.tags("SPAN");
	for (var i = 0; i < spans.length; i++)
	{
		var span = spans[i];
		if (span.className == "lang")
		{
			var tokens = span.innerText.split(",");
			for (var t = 0; t < tokens.length; t++)
			{
				var token = tokens[t];
				var id = token.match(/([A-Za-z].*[A-Za-z+#0-9]+)/);
				if (id && !IsArrayElement(languages, id[1]))
					languages[languages.length] = id[1];
			}
		}
	}
	languages.sort();
	return languages;
}

function DetermineLanguage()
{
	try
	{
		for (var i = 1; i < window.external.ContextAttributes.Count; i++)
		{
			if (window.external.ContextAttributes(i).Name.toUpperCase()=="DEVLANG")
			{
				var attributes = window.external.ContextAttributes(i).Values.toArray();
				var language = attributes[0].toUpperCase();
				if (language != null)
				{
					if (language.indexOf("VB")!=-1)
						return "Visual Basic";
					if (language.indexOf("VC")!=-1)
						return "C++";
					if (language.indexOf("CSHARP")!=-1)
						return "C#";
				}
			}
		}
	}
	catch(e)
	{
	}

	try
	{
		var language = window.external.Help.FilterQuery.toUpperCase();
		if (language.indexOf("VISUAL BASIC") !=- 1)
			return "Visual Basic";
		if (language.indexOf("VISUAL C++") !=- 1)
			return "C++";
		if (language.indexOf("C#") != -1)
			return "C#";
	}
	catch(e)
	{
	}

	return null;
}

/////////////////////////////////////////////////////////////////////////////
// Reftips (parameter popups)
/////////////////////////////////////////////////////////////////////////////

function InitializeReftips()
{
	var pres = document.all.tags("PRE");
	var dls = document.all.tags("DL");
	if (!pres || !dls)
		return;

	var list = 0;

	var lastTip = -1;
	for (var i = 0; i < pres.length; i++)
	{
		var pre = pres[i];
		if (pre.className == "syntax")
		{
			while (list < dls.length && dls[list].sourceIndex < pre.sourceIndex)
				list++;
			if (list >= dls.length)
				break;

			InitializeSyntaxTip(pre, dls[list]);
			lastTip = i;
		}
	}

	if (lastTip >= 0)
	{
		var last = pres[lastTip];
		if (last.parentElement.tagName == "DIV")
			last = last.parentElement;
		last.insertAdjacentHTML('afterEnd', '<DIV id="reftip" class="reftip" style="position:absolute;visibility:hidden;overflow:visible;"></DIV>');
	}
}

function InitializeSyntaxTip(pre, dl)
{
	var syntax = pre.outerHTML;

	var ichStart = syntax.indexOf('>', 0) + 1;

	var terms = dl.children.tags("DT");
	if (terms)
	{
		for (var i = 0; i < terms.length; i++)
		{
			var term = terms[i];
			var html = term.innerHTML;

			var words = term.innerText.replace(/\[.+\]/g, " ").replace(/,/g, " ").split(" ");
			for (var iWord = 0; iWord < words.length; iWord++)
			{
				var word = words[iWord];

				if (word.length > 0 && html.indexOf(word, 0) < 0)
					word = word.replace(/:.+/, "");

				if (word.length > 0)
				{
					var matchPos = FindTerm(syntax, ichStart, word);
					while (matchPos > 0)
					{
						if (!IsLinkText(syntax.substring(matchPos)))
						{
							var tag = '<A href="" onclick="ShowTip(this)" class="synParam">' + word + '</A>';
							syntax = syntax.slice(0, matchPos) + tag + syntax.slice(matchPos + word.length);
							matchPos = FindTerm(syntax, matchPos + tag.length, word);
						}
						else
						{
							matchPos = FindTerm(syntax, matchPos + word.length, word);
						}
					}
				}
			}
		}
	}

	pre.outerHTML = syntax;
}
function ShowTip(link)
{
	window.event.returnValue = false;
	window.event.cancelBubble = true;

	ResetGUI();

	var tip = document.all.reftip;
	if (!tip || !link)
		return;

	tip.style.visibility = "hidden";
	tip.style.pixelWidth = 260;
	tip.style.pixelHeight = 24;

	var term = null;
	var definition = null;

	var dls = document.all.tags("DL");
	for (var i = 0; i < dls.length; i++)
	{
		var dl = dls[i];
		if (dl.sourceIndex > link.sourceIndex)
		{
			var iMax = dl.children.length - 1;
			for (var iElem = 0; iElem < iMax; iElem++)
			{
				var dt = dl.children[iElem];
				if (dt.tagName == "DT" && dt.style.display != "none")
				{
					if (FindTerm(dt.innerText, 0, link.innerText) >= 0)
					{
						var dd = dl.children[iElem + 1];
						if (dd.tagName == "DD")
						{
							term = dt;
							definition = dd;
						}
						break;
					}
				}
			}
			break;
		}
	}

	if (definition)
	{
		window.linkElement = link;
		window.linkTarget = term;
		tip.innerHTML = '<DL><DT>' + term.innerHTML + '</DT><DD>' + definition.innerHTML + '</DD></DL>';
		window.setTimeout("MoveTip()", 0);
	}
}

function MoveTip()
{
	var tip = document.all.reftip;
	var link = window.linkElement;
	if (!tip || !link)
		return;

	var w = tip.offsetWidth;
	var h = tip.offsetHeight;

	if (w > tip.style.pixelWidth)
	{
		tip.style.pixelWidth = w;
		window.setTimeout("MoveTip()", 0);
		return;
	}

	var maxw = document.body.clientWidth - 20;
	var maxh = document.body.clientHeight - 200;

	if (h > maxh)
	{
		if (w < maxw)
		{
			w = w * 3 / 2;
			tip.style.pixelWidth = (w < maxw) ? w : maxw;
			window.setTimeout("MoveTip()", 0);
			return;
		}
	}

	var x, y;

	var linkLeft = link.offsetLeft - document.body.scrollLeft;
	var linkRight = linkLeft + link.offsetWidth;

	try
	{
		var linkTop = link.offsetTop - nstext.scrollTop + nstext.offsetTop;
	}
	catch(e)
	{
		var linkTop = link.offsetTop;
	}

	var linkBottom = linkTop + link.offsetHeight + 4;

	var cxMin = link.offsetWidth - 24;
	if (cxMin < 16)
		cxMin = 16;

	if ((linkLeft + cxMin + w <= maxw) && (h+linkTop <= maxh + 150))
	{
		x = linkLeft;
		y = linkBottom;
	}
	if ((linkLeft + cxMin + w <= maxw) && (h+linkTop > maxh + 150))
	{
		x = maxw - w;
		if (x > linkRight + 8)
			x = linkRight + 8;
		x = linkLeft;
		y = linkTop-h;
	}
	if ((linkLeft + cxMin + w >= maxw) && (h+linkTop <= maxh + 150))
	{
		x = linkRight - w;
		if (x < 0)
			x = 0;
		y = linkBottom;
	}
	if ((linkLeft + cxMin + w >= maxw) && (h+linkTop > maxh + 150))
	{
		x = linkRight - w;
		if (x < 0)
			x = 0;
		y = linkTop - h;
		if (y < 0)
			y = 0;
	}
	link.style.background = "#CCCCCC";
	tip.style.pixelLeft = x + document.body.scrollLeft;
	tip.style.pixelTop = y;
	tip.style.visibility = "visible";
	CreateDropShadow(tip);
}

function HideTip()
{
	var tip = document.all.reftip;
	var link = window.linkElement;
	if (!tip || !link)
		return;

	window.linkElement.style.background = "";
	window.linkElement = null;

	tip.style.visibility = "hidden";
	tip.innerHTML = "";
	RemoveDropShadow();
}

function FindTerm(syntax, pos, term)
{
	var matchPos = syntax.indexOf(term, pos);
	while (matchPos >= 0)
	{
		var prev = (matchPos == 0) ? '\0' : syntax.charAt(matchPos - 1);
		var next = syntax.charAt(matchPos + term.length);
		if (!isalnum(prev) && !isalnum(next) && !IsInTag(syntax, matchPos))
		{
			var commentPos = syntax.indexOf("/*", pos);
			while (commentPos >= 0)
			{
				if (commentPos > matchPos)
				{
					commentPos = -1;
					break;
				}
				var ichEnd = syntax.indexOf("*/", commentPos);
				if (ichEnd < 0 || ichEnd > matchPos)
					break;
				commentPos = syntax.indexOf("/*", ichEnd);
			}
			if (commentPos < 0)
			{
				commentPos = syntax.indexOf("//", pos);
				var newPos = 0;
				if (commentPos >= 0)
				{
					while (IsInTag(syntax, commentPos))
					{
						newPos = commentPos + 1;
						commentPos = syntax.indexOf("//", newPos);
						if (commentPos < 0)
							break;
					}
					while (commentPos >= 0)
					{
						if (commentPos > matchPos)
						{
							commentPos = -1;
							break;
						}
						var ichEnd = syntax.indexOf("\n", commentPos);
						if (ichEnd < 0 || ichEnd > matchPos)
							break;
						commentPos = syntax.indexOf("//", ichEnd);
					}
				}
			}
			if (commentPos < 0)
				break;
		}
		matchPos = syntax.indexOf(term, matchPos + term.length);
	}
	return matchPos;
}

function IsLinkText(html)
{
	return html.indexOf("<") == html.toLowerCase().indexOf("<\/a>");
}

function IsInTag(html, pos)
{
	return html.lastIndexOf('<', pos) > html.lastIndexOf('>', pos);
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
// Drop shadow functions
/////////////////////////////////////////////////////////////////////////////

function CreateDropShadow(e)
{
	if (false)
	{
		g_DropShadowRef = e;

		for (var i = g_DropShadowDepth; i > 0; i--)
		{
			var div = document.createElement("DIV");
			div.style.position = "absolute";
			div.style.left = (e.style.posLeft + i) + "px";
			div.style.top = (e.style.posTop + i) + "px";
			div.style.width = e.offsetWidth + "px";
			div.style.height = e.offsetHeight + "px";
			div.style.zIndex = e.style.zIndex - i;
			div.style.backgroundColor = g_DropShadowColor;
			div.style.filter = "alpha(opacity=" + ((1 - i / (i + 1)) * 50) + ")";
			e.insertAdjacentElement("afterEnd", div);
			g_DropShadows[g_DropShadows.length] = div;
		}
	}
}

function RemoveDropShadow()
{
	if (false)
	{
		for (var i = 0; i < g_DropShadows.length; i++)
			g_DropShadows[i].removeNode(true);
		g_DropShadows = new Array();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////

function GetNextSibling(e)
{
    return document.all(e.sourceIndex + e.children.length + 1);
}

function BeginsWith(s1, s2)
{
	return s1.toLowerCase().substring(0, s2.length) == s2.toLowerCase();
}

function IsArrayElement(array, element)
{
	for (var i = 0; i < array.length; i++)
	{
		if (array[i] == element)
			return true;
	}
	return false;
}

function isalnum(ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '_') || (ch == '-');
}

