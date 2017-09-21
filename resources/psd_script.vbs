Set app = CreateObject("Photoshop.Application")
app.DisplayDialogs = 3

Do While app.Documents.Count
	app.ActiveDocument.Close()
Loop

Set fso = CreateObject( "Scripting.FileSystemObject" )
Set folder = fso.GetFolder( "C:\Users\Pietro_Alberti\Documents\Code\Cabanon_Server\to_print\" )

For Each f in folder.Files
	app.Open f.Path
	app.DoAction "BWAndCurves", "Cabanon"
	app.DoAction "SaveAndClose", "Cabanon"
Next