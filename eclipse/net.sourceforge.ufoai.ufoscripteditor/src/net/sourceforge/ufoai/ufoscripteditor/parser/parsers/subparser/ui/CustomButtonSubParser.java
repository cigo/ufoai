package net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.ui;

import net.sourceforge.ufoai.ufoscripteditor.parser.IParserContext;
import net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.IUFOSubParser;
import net.sourceforge.ufoai.ufoscripteditor.parser.parsers.subparser.IUFOSubParserFactory;

public class CustomButtonSubParser extends ButtonSubParser {
	public static final IUFOSubParserFactory FACTORY = new UFONodeParserFactoryAdapter() {
		@Override
		public String getID() {
			return "custombutton";
		}

		@Override
		public IUFOSubParser create(IParserContext ctx) {
			return new CustomButtonSubParser(ctx);
		}

	};

	@Override
	public IUFOSubParserFactory getNodeSubParserFactory() {
		return FACTORY;
	}

	public CustomButtonSubParser(IParserContext ctx) {
		super(ctx);
		// IUFOSubParserFactory factory = getNodeSubParserFactory();
		// Registration for event properties

	}
}
