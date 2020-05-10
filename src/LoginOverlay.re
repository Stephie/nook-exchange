module Styles = {
  open Css;
  let overlay =
    style([
      position(fixed),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      display(flexBox),
      alignItems(center),
      justifyContent(center),
      zIndex(1),
    ]);
  let backdrop =
    style([
      position(absolute),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      backgroundColor(hex("6f8477d0")),
    ]);
  let root =
    style([
      backgroundColor(hex("ffffff")),
      padding2(~v=px(32), ~h=px(12)),
      borderRadius(px(4)),
      position(relative),
      maxWidth(px(448)),
      boxSizing(borderBox),
      width(pct(90.)),
      boxShadow(Shadow.box(~spread=px(12), rgba(0, 0, 0, 0.1))),
      overflow(auto),
      maxHeight(vh(100.)),
      media(
        "(max-width: 540px)",
        [paddingTop(px(24)), paddingBottom(px(24))],
      ),
    ]);
  let body = style([maxWidth(px(320)), margin2(~v=zero, ~h=auto)]);
  let input =
    style([
      backgroundColor(rgba(0, 0, 0, 0.05)),
      border(px(1), solid, transparent),
      borderRadius(px(4)),
      padding2(~v=px(10), ~h=px(12)),
      boxSizing(borderBox),
      outlineStyle(none),
      fontSize(px(16)),
      width(pct(100.)),
      marginBottom(px(16)),
      transition(~duration=200, "all"),
      focus([
        backgroundColor(hex("ffffff")),
        borderColor(rgba(0, 0, 0, 0.15)),
        boxShadow(Shadow.box(~spread=px(4), rgba(0, 0, 0, 0.05))),
      ]),
    ]);
  let submitBar =
    style([display(flexBox), alignItems(center), justifyContent(flexEnd)]);
  let submitButton =
    style([
      backgroundColor(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      color(Colors.white),
      cursor(pointer),
      marginLeft(px(16)),
      padding2(~v=px(10), ~h=px(16)),
      fontSize(px(16)),
      transition(~duration=200, "all"),
      disabled([opacity(0.5)]),
    ]);
  let divider =
    style([
      backgroundColor(hex("e0e0e0")),
      height(px(1)),
      width(pct(100.)),
      margin2(~v=px(24), ~h=zero),
    ]);
  let terms =
    style([color(hex("a0a0a0")), fontSize(px(12)), marginTop(px(32))]);
  let registerTitle =
    style([fontSize(px(20)), textAlign(center), marginBottom(px(24))]);
  let blurb = style([marginBottom(px(16))]);
  [@bs.module "./assets/close.png"] external closePng: string = "default";
  let closeButton =
    style([
      backgroundColor(transparent),
      backgroundImage(url(closePng)),
      backgroundSize(`size((px(16), px(16)))),
      backgroundRepeat(noRepeat),
      backgroundPosition(center),
      borderWidth(zero),
      padding(zero),
      cursor(pointer),
      height(px(48)),
      width(px(48)),
      position(absolute),
      top(zero),
      right(zero),
      opacity(0.5),
      hover([opacity(1.)]),
    ]);
  let urlPreview =
    style([
      fontSize(px(12)),
      letterSpacing(pxFloat(0.3)),
      color(hex("b0b0b0")),
      marginBottom(px(8)),
    ]);
  let url =
    style([
      border(px(2), dashed, Colors.lightGreen),
      textAlign(center),
      borderRadius(px(4)),
      padding(px(16)),
      margin2(~v=px(16), ~h=zero),
    ]);
  let successMessage = style([flexGrow(1.), color(Colors.green)]);
  let errorMessage =
    style([marginTop(px(-10)), marginBottom(px(16)), color(Colors.red)]);
};

module PasswordReset = {
  type status =
    | Success
    | Error(string);

  [@react.component]
  let make = () => {
    let (email, setEmail) = React.useState(() => "");
    let (isSubmitting, setIsSubmitting) = React.useState(() => false);
    let (status, setStatus) = React.useState(() => None);

    let onSubmit = e => {
      ReactEvent.Form.preventDefault(e);
      setIsSubmitting(_ => true);
      {
        let%Repromise.JsExn response =
          Fetch.fetchWithInit(
            Constants.bapiUrl ++ "/password-reset",
            Fetch.RequestInit.make(
              ~method_=Post,
              ~body=
                Fetch.BodyInit.make(
                  Js.Json.stringify(
                    Json.Encode.object_([("email", Js.Json.string(email))]),
                  ),
                ),
              ~headers=
                Fetch.HeadersInit.make({
                  "X-Client-Version": Constants.gitCommitRef,
                  "Content-Type": "application/json",
                }),
              ~mode=CORS,
              (),
            ),
          );
        let%Repromise _ =
          if (Fetch.Response.status(response) < 300) {
            setStatus(_ => Some(Success));
            Analytics.Amplitude.logEventWithProperties(
              ~eventName="Reset Password Request Success",
              ~eventProperties={"email": email},
            );
            Promise.resolved();
          } else {
            let%Repromise.JsExn text = Fetch.Response.text(response);
            setStatus(_ => Some(Error(text)));
            Analytics.Amplitude.logEventWithProperties(
              ~eventName="Reset Password Request Failure",
              ~eventProperties={"email": email, "error": text},
            );
            Promise.resolved();
          };
        setIsSubmitting(_ => false);
        Promise.resolved();
      }
      |> ignore;
    };

    <form onSubmit>
      <div className=Styles.registerTitle>
        {React.string("Reset your password")}
      </div>
      <input
        type_="text"
        placeholder="Account email"
        value=email
        onChange={e => {
          let value = ReactEvent.Form.target(e)##value;
          setEmail(_ => value);
        }}
        className=Styles.input
      />
      {switch (status) {
       | Some(Error(error)) =>
         <div className=Styles.errorMessage> {React.string(error)} </div>
       | _ => React.null
       }}
      <div className=Styles.submitBar>
        {status == Some(Success)
           ? <div className=Styles.successMessage>
               {React.string("Check your email!")}
             </div>
           : React.null}
        <button
          type_="submit"
          disabled={isSubmitting || status == Some(Success)}
          className=Styles.submitButton>
          {React.string("Send reset link")}
        </button>
      </div>
    </form>;
  };
};

type registerStatus =
  | Success
  | Error(string);

type screen =
  | Login
  | Register
  | PasswordReset;

[@react.component]
let make = (~onClose) => {
  let (screen, setScreen) = React.useState(() => Register);

  let (username, setUsername) = React.useState(() => "");
  let (email, setEmail) = React.useState(() => "");
  let (password, setPassword) = React.useState(() => "");
  let (isSubmitting, setIsSubmitting) = React.useState(() => false);

  React.useEffect0(() => {
    Analytics.Amplitude.logEvent(~eventName="Registration Viewed");
    None;
  });

  let (registerStatus, setRegisterStatus) = React.useState(() => None);
  let onLoginSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    {
      setIsSubmitting(_ => true);
      let%Repromise result = UserStore.login(~username, ~password);
      switch (result) {
      | Ok(_) =>
        onClose();
        Analytics.Amplitude.logEvent(~eventName="Login Succeeded");
      | Error(_) =>
        setRegisterStatus(_ =>
          Some(Error("Login failed. Please try again."))
        );
        Analytics.Amplitude.logEvent(~eventName="Login Failed");
        setIsSubmitting(_ => false);
      };
      Promise.resolved();
    }
    |> ignore;
  };

  let onRegisterSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    {
      setIsSubmitting(_ => true);
      let%Repromise result = UserStore.register(~username, ~email, ~password);
      switch (result) {
      | Ok(_) =>
        setRegisterStatus(_ => Some(Success));
        Analytics.Amplitude.logEventWithProperties(
          ~eventName="Registration Succeeded",
          ~eventProperties={"username": username, "email": email},
        );
      | Error(error) =>
        setRegisterStatus(_ => Some(Error(error)));
        Analytics.Amplitude.logEventWithProperties(
          ~eventName="Registration Failed",
          ~eventProperties={"error": error},
        );
      };
      setIsSubmitting(_ => false);
      Promise.resolved();
    }
    |> ignore;
  };

  let usernameRef = React.useRef(Js.Nullable.null);
  React.useEffect0(() => {
    open Webapi.Dom;
    let usernameInput =
      Utils.getElementForDomRef(usernameRef)->Element.unsafeAsHtmlElement;
    HtmlElement.focus(usernameInput);
    None;
  });

  <div className=Styles.overlay>
    <div className=Styles.backdrop onClick={_ => onClose()} />
    <div className=Styles.root>
      <div className=Styles.body>
        {switch (screen) {
         | Login =>
           <div>
             <form onSubmit=onLoginSubmit>
               <div className=Styles.registerTitle>
                 {React.string("Welcome back!")}
               </div>
               <input
                 type_="text"
                 placeholder="Username"
                 value=username
                 onChange={e => {
                   let value = ReactEvent.Form.target(e)##value;
                   setUsername(_ => value);
                 }}
                 className=Styles.input
               />
               <input
                 type_="password"
                 placeholder="Password"
                 value=password
                 onChange={e => {
                   let value = ReactEvent.Form.target(e)##value;
                   setPassword(_ => value);
                 }}
                 className=Styles.input
               />
               <div className=Styles.errorMessage>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setScreen(_ => PasswordReset);
                   }}>
                   {React.string("Forgot password?")}
                 </a>
               </div>
               {switch (registerStatus) {
                | Some(Error(error)) =>
                  <div className=Styles.errorMessage>
                    {React.string(error)}
                  </div>
                | _ => React.null
                }}
               <div className=Styles.submitBar>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setScreen(_ => Register);
                   }}>
                   {React.string("Need an account?")}
                 </a>
                 <button
                   type_="submit"
                   disabled=isSubmitting
                   className=Styles.submitButton>
                   {React.string("Login")}
                 </button>
               </div>
             </form>
           </div>
         | Register =>
           registerStatus == Some(Success)
             ? <div>
                 <div className=Styles.registerTitle>
                   {React.string("Your account is created!")}
                 </div>
                 <div className=Styles.blurb>
                   {React.string(
                      "Add items to profile and share it with others.",
                    )}
                 </div>
                 <div className=Styles.url>
                   <Link path={"/u/" ++ username}>
                     {React.string("nook.exchange/u/" ++ username)}
                   </Link>
                 </div>
                 <div className=Styles.submitBar>
                   <button
                     onClick={_ => onClose()} className=Styles.submitButton>
                     {React.string("Okay!")}
                   </button>
                 </div>
               </div>
             : <div>
                 <div className=Styles.registerTitle>
                   {React.string("Register an account")}
                 </div>
                 <div className=Styles.blurb>
                   {React.string(
                      "Create and share lists of Animal Crossing items!",
                    )}
                 </div>
                 <div className=Styles.urlPreview>
                   {React.string("nook.exchange/u/" ++ username)}
                 </div>
                 <div>
                   <form onSubmit=onRegisterSubmit>
                     <input
                       type_="text"
                       placeholder="Username"
                       value=username
                       onChange={e => {
                         let value = ReactEvent.Form.target(e)##value;
                         setUsername(_ => value);
                       }}
                       ref={ReactDOMRe.Ref.domRef(usernameRef)}
                       className=Styles.input
                     />
                     <input
                       type_="text"
                       placeholder="Email (Optional for account recovery)"
                       value=email
                       onChange={e => {
                         let value = ReactEvent.Form.target(e)##value;
                         setEmail(_ => value);
                       }}
                       className=Styles.input
                     />
                     <input
                       type_="password"
                       placeholder="Password"
                       value=password
                       onChange={e => {
                         let value = ReactEvent.Form.target(e)##value;
                         setPassword(_ => value);
                       }}
                       className=Styles.input
                     />
                     {switch (registerStatus) {
                      | Some(Error(error)) =>
                        <div className=Styles.errorMessage>
                          {React.string(error)}
                        </div>
                      | _ => React.null
                      }}
                     <div className=Styles.submitBar>
                       <a
                         href="#"
                         onClick={e => {
                           ReactEvent.Mouse.preventDefault(e);
                           setScreen(_ => Login);
                         }}>
                         {React.string("Login instead?")}
                       </a>
                       <button
                         type_="submit"
                         className=Styles.submitButton
                         disabled={
                           Js.String.length(password) < 4 || isSubmitting
                         }>
                         {React.string("Register")}
                       </button>
                     </div>
                     <div className=Styles.terms>
                       {React.string(
                          "By registering, you agree to Nook Exchange's ",
                        )}
                       <a href="/terms" target="_blank">
                         {React.string("Terms of Service")}
                       </a>
                       {React.string(" and ")}
                       <a href="/privacy" target="_blank">
                         {React.string("Privacy Policy")}
                       </a>
                       {React.string(".")}
                     </div>
                   </form>
                 </div>
               </div>
         | PasswordReset => <PasswordReset />
         }}
      </div>
      <button onClick={_ => onClose()} className=Styles.closeButton />
    </div>
  </div>;
};