/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
#define NULL 0
#include "helpfile.h"

char *helpname[MAXHELPNAME] =
{
    "ConsoleCommands#help:Console",
    "ConsoleCommands#help:Console.Window",
    "ConsoleCommands#help:Console.EditOp",
    "ConsoleCommands#help:Console.Structure",
    "ConsoleCommands#help:Console.Selection",
    "ConsoleCommands#help:Console.Misc.",
    "ConsoleCommands#help:Console.Version",
    "ConsoleCommands#help:Console.Quit",
    NULL, NULL,
    "Basics#help:Document",
    "MoreOnWindows#help:Document.Load",
    "MoreOnWindows#help:Document.Save",
    "MoreOnWindows#help:Document.Rename",
    "MoreOnWindows#help:Document.Output",
    "MoreOnWindows#help:Document.Include",
    "MoreOnWindows#help:Document.Done",
    "TextEditing",    
    NULL, NULL,
    "Basics#help:File Selector",
    "MoreOnWindows#help:File Selector.Ok",
    "MoreOnWindows#help:File Selector.Rescan",
    "MoreOnWindows#help:File Selector.Cancel",
    "MoreOnWindows#help:File Selector.Selection",
    "MoreOnWindows#help:File Selector.Mask",
    NULL, NULL, NULL, NULL,
    "Basics#help:Stencils",
    "MoreOnWindows#help:Stencils.Load",
    "MoreOnWindows#help:Stencils.Save",
    "MoreOnWindows#help:Stencils.Rename",
    "MoreOnWindows#help:Stencils.Define",
    "MoreOnWindows#help:Stencils.Done",
    NULL, NULL, NULL, NULL,
    "Basics#help:Define",
    "StencilsandTemplates#help:Define.New",
    "StencilsandTemplates#help:Define.Update",
    "StencilsandTemplates#help:Define.Remove",
    "StencilsandTemplates#help:Define.Double",
    "StencilsandTemplates#help:Define.Done",
    "StencilsandTemplates#help:Define.Prec.",
    "StencilsandTemplates#help:Define.Space",
    "StencilsandTemplates#help:Define.Kind",
    "VersionDefinition#help:Define.Place",
    "VersionDefinition#help:Define.Tabs",
    "VersionDefinition#help:Define.Mode",
    "VersionDefinition#help:Define.Font",
    "VersionDefinition#help:Define.Size",
    "VersionDefinition#help:Define.Misc",
    "StencilsandTemplates#help:Define.Name",
    "StencilsandTemplates#help:Define.Help file",
    "VersionDefinition#help:Define.Version",
    NULL, NULL,
    "Basics#help:Symbol",
    "MoreOnWindows#help:Symbol.Prev",
    "MoreOnWindows#help:Symbol.Next",
    "MoreOnWindows#help:Symbol.Done",
    NULL, NULL, NULL, NULL, NULL, NULL,
    "Basics#help:Buffer",
    "Basics#help:Buffer.Kill",
    "Basics#help:Buffer.Done",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    "Basics#help:Find",
    "FindAndReplace#help:FindAndReplace.Find",
    "FindAndReplace#help:FindAndReplace.Stack",
    "FindAndReplace#help:FindAndReplace.Done",
    "FindAndReplace#help:FindAndReplace.Upper",
    "FindAndReplace#help:FindAndReplace.Lower",
    "FindAndReplace#help:FindAndReplace.Find.pinup",
    "FindAndReplace#help:FindAndReplace.Stack.pinup",
    NULL, NULL,
    "Basics#help:Default",
    "Defaults#help:Default.Set",
    "Defaults#help:Default.Save",
    "Defaults#help:Default.Done",
    "Defaults#help:Default.Screen",
    "Defaults#help:Default.Latex",
    "Defaults#help:Default.Directory",
    "Defaults#help:Default.Fonts",
    "Defaults#help:Default.Keys",
    "Defaults#help:Default.Save Time",
    "Defaults#help:Default.Left margin",
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL,
    "Basics#help:button",
    "Basics#help:Scrolling",
    "Basics#help:PinUpMenus",
    "Basics#help:Keyboard",
    "Basics#help:Getstring",
    "Basics#help:Pop-Ups",
    NULL, NULL, NULL, NULL,
    "ConsoleCommands#help:Console.Window.pinup",
    "ConsoleCommands#help:Console.EditOp.pinup",
    "ConsoleCommands#help:Console.Structure.pinup",
    "ConsoleCommands#help:Console.Selection.pinup",
    "ConsoleCommands#help:Console.Misc.pinup",
    "ConsoleCommands#help:Console.Version.pinup",
    "ConsoleCommands#help:Console.Quit.pinup",
};
