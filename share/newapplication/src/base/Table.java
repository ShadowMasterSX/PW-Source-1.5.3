package com.wanmei.db;

import java.io.*;
import java.util.*;
import org.w3c.dom.*;
import java.sql.*;

class Table
{
	private Application application;
	private String name;
	private List<TableColumn    >     column_list = new ArrayList<TableColumn    >();
	private List<TablePrimaryKey> primarykey_list = new ArrayList<TablePrimaryKey>();
	private List<TableIndex     >      index_list = new ArrayList<TableIndex     >();
	private List<TableInsert    >     insert_list = new ArrayList<TableInsert    >();
	private List<TableUpdate    >     update_list = new ArrayList<TableUpdate    >();
	private List<TableDelete    >     delete_list = new ArrayList<TableDelete    >();

	Table( Application app, Element elem ) throws Exception
	{
		application = app;
		name = elem.getAttribute( "name" );

		List<Element>     column_elements = new ArrayList<Element>();
		List<Element> primarykey_elements = new ArrayList<Element>();
		List<Element>      index_elements = new ArrayList<Element>();
		List<Element>     insert_elements = new ArrayList<Element>();
		List<Element>     update_elements = new ArrayList<Element>();
		List<Element>     delete_elements = new ArrayList<Element>();

		NodeList nl = elem.getChildNodes();
		for ( int i = 0; i < nl.getLength(); i++ )
		{
			Node node = nl.item(i);
			if ( node.getNodeType() != Node.ELEMENT_NODE ) continue;
			Element e = (Element)node;
			String nodeName = e.getNodeName();
			if      ( nodeName.compareToIgnoreCase( "column"     ) == 0 )     column_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "primarykey" ) == 0 ) primarykey_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "index"      ) == 0 )      index_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "insert"     ) == 0 )     insert_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "update"     ) == 0 )     update_elements.add( e );
			else if ( nodeName.compareToIgnoreCase( "delete"     ) == 0 )     delete_elements.add( e );
		}

		for ( Element e :     column_elements )     column_list.add ( new TableColumn    ( this, e ) );
		for ( Element e : primarykey_elements ) primarykey_list.add ( new TablePrimaryKey( this, e ) );
		for ( Element e :      index_elements )      index_list.add ( new TableIndex     ( this, e ) );
		for ( Element e :     insert_elements )     insert_list.add ( new TableInsert    ( this, e ) );
		for ( Element e :     update_elements )     update_list.add ( new TableUpdate    ( this, e ) );
		for ( Element e :     delete_elements )     delete_list.add ( new TableDelete    ( this, e ) );
	}

	String DDL()
	{
		StringBuffer sb = new StringBuffer();
		sb.append( "CREATE TABLE " + name + " (\n" );
		for ( TableColumn i : column_list )
		{
			sb.append("\t");
			sb.append( i.DDL() );
			sb.append(",\n");
		}
		sb = sb.deleteCharAt(sb.lastIndexOf(","));
		sb.append( ")\n" );

		for ( TablePrimaryKey i : primarykey_list )
		{
			sb.append( i.DDL() );
			sb.append( "\n");
		}

		for ( TableIndex i : index_list )
		{
			sb.append( i.DDL() );
			sb.append( "\n");
		}
		return sb.toString();
	}

	String DML()
	{
		StringBuffer sb = new StringBuffer();
		for ( TableInsert i : insert_list ) sb.append( i.DML() ).append( "\n" );
		for ( TableUpdate i : update_list ) sb.append( i.DML() ).append( "\n" );
		for ( TableDelete i : delete_list ) sb.append( i.DML() ).append( "\n" );
		return sb.toString();
	}

	void verify() throws Exception
	{
		if ( primarykey_list.size() > 1 )
			throw new Exception ( "[" + DDL() + "] More than 1 PrimaryKey" );
		for ( TablePrimaryKey i : primarykey_list ) i.verify();
		for ( TableIndex      i :      index_list ) i.verify();
		for ( TableInsert     i :     insert_list ) i.verify();
		for ( TableUpdate     i :     update_list ) i.verify();
		for ( TableDelete     i :     delete_list ) i.verify();
	}

	void makeHead( PrintStream ps, String sql, String method_name ) throws Exception
	{
		String beanname = application.translateBeanName(name);

		ps.println( "{" );
		ps.println( "	int r = -1;" );

		ps.println( "	CounterHolder.storage.increment(\"" + beanname + "." + method_name + "\");");
		ps.println( "	try" );
		ps.println( "	{" );
		ps.println( "		java.sql.PreparedStatement ps = ctx.prepareStatement(\"" + sql + "\");" );
	}

	static void makeTail( PrintStream ps ) throws Exception
	{
		ps.println( "		r = ps.executeUpdate();" );
		ps.println( "	}" );
		ps.println( "	catch (Exception e)" );
		ps.println( "	{" );
		ps.println( "		ctx.setException(e);" );
		ps.println( "	}" );
		ps.println( "	finally" );
		ps.println( "	{" );
		ps.println( "		ctx.doFinally();" );
		ps.println( "	}" );
		ps.println( "	return r;" );
		ps.println( "}" );
	}

	String columnList()
	{
		StringBuffer sb = new StringBuffer("(");
		for ( TableColumn i : column_list )
			sb.append ( i.getJava_type() + " _" + i.getName() + "," );
		sb.setCharAt(sb.lastIndexOf(","), ')');
		return sb.toString();
	}

	void makeBeans( String beans, String beanbase ) throws Exception
	{
		PrintStream ps = application.makeClassHead( beans, beanbase, name, "table" );

		String beanname = application.translateBeanName(name);
		ps.println( "public " + beanname + columnList() );
		ps.println( "{" );
		for ( TableColumn i : column_list )
		ps.println( "	" + i.getName() + " = _" + i.getName() + ";" );
		ps.println( "}" );

		for ( TableColumn i : column_list ) i.makeBeans( ps );
		for ( TableInsert i : insert_list ) i.makeBeans( ps );
		ps.println( "public int insert(Context ctx)" );
		ps.println( "{" );
		// default insert
		ps.println( "	CounterHolder.storage.increment(\"" + beanname + ".insert\");");

		ps.println( "	List<String> columnName = new ArrayList<String>();" );
		ps.println( "	List<Object> columnObj = new ArrayList<Object>();" );
		for ( TableColumn i : column_list )
		ps.println( "	if(" + i.getName() + "!=null) { columnName.add(\"" + i.getName() + "\"); columnObj.add(" + i.getName() + ");}" );
		ps.println( "	int r = -1;" );
		ps.println( "	try" );
		ps.println( "	{" );
		ps.println( "		r = ctx.insert( \"" + name + "\", columnName, columnObj );" );
		ps.println( "	}" );
		ps.println( "	catch (Exception e)" );
		ps.println( "	{" );
		ps.println( "		ctx.setException(e);" );
		ps.println( "	}" );
		ps.println( "	finally" );
		ps.println( "	{" );
		ps.println( "		ctx.doFinally();" );
		ps.println( "	}" );
		ps.println( "	return r;" );
		ps.println( "}" );

		ps.println( "public int insert()" );
		ps.println( "{" );
		ps.println( "	return insert(ApplicationThreadFrameWork.getContext());");
		ps.println( "}" );

		for ( TableUpdate i : update_list ) i.makeBeans( ps );
		for ( TableDelete i : delete_list ) i.makeBeans( ps );
		application.makeClassTail( ps );
	}

	String getName() { return name; }
	Application getApplication() { return application; }
	List<TableColumn> getColumnList() { return column_list; }

	TableColumn getTableColumn( String name )
	{
		for ( TableColumn i : column_list )
			if ( name.compareTo( i.getName() ) == 0 )
				return i;
		return null;
	}
}

